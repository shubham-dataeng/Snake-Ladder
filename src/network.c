#include "network.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/select.h>
#include <errno.h>

/* ========== Server Functions ========== */

bool server_init(ServerContext *ctx, int port) {
	memset(ctx, 0, sizeof(ServerContext));
	ctx->port = port;
	ctx->num_connected = 0;
	ctx->start_time = time(NULL);

	ctx->server_socket = socket(AF_INET, SOCK_STREAM, 0);
	if (ctx->server_socket < 0) {
		perror("socket");
		return false;
	}

	/* Allow reuse of address */
	int reuse = 1;
	if (setsockopt(ctx->server_socket, SOL_SOCKET, SO_REUSEADDR,
				   &reuse, sizeof(reuse)) < 0) {
		perror("setsockopt");
		close(ctx->server_socket);
		return false;
	}

	struct sockaddr_in addr;
	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = htonl(INADDR_ANY);
	addr.sin_port = htons(port);

	if (bind(ctx->server_socket, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
		perror("bind");
		close(ctx->server_socket);
		return false;
	}

	if (listen(ctx->server_socket, 2) < 0) {
		perror("listen");
		close(ctx->server_socket);
		return false;
	}

	printf("[Server] Listening on port %d\n", port);
	return true;
}

bool server_accept_clients(ServerContext *ctx) {
	if (ctx->num_connected >= 2) {
		printf("[Server] Already have 2 clients\n");
		return true;
	}

	struct sockaddr_in addr;
	socklen_t addr_len = sizeof(addr);

	int client_socket = accept(ctx->server_socket, (struct sockaddr *)&addr, &addr_len);
	if (client_socket < 0) {
		if (errno == EAGAIN || errno == EWOULDBLOCK) {
			return true; /* No client waiting */
		}
		perror("accept");
		return false;
	}

	int idx = ctx->num_connected;
	ctx->clients[idx].socket_fd = client_socket;
	ctx->clients[idx].addr = addr;
	ctx->clients[idx].player_id = idx + 1;
	ctx->clients[idx].connected = true;
	ctx->clients[idx].last_heartbeat = time(NULL);
	ctx->num_connected++;

	printf("[Server] Player %d connected from %s:%d\n", 
	       idx + 1, inet_ntoa(addr.sin_addr), ntohs(addr.sin_port));

	return true;
}

bool server_send_message(ServerContext *ctx, int client_idx, const NetworkMessage *msg) {
	if (client_idx < 0 || client_idx >= 2) return false;
	if (!ctx->clients[client_idx].connected) return false;

	char buffer[MAX_MESSAGE_LEN];
	int len = 0;
	if (!message_serialize(msg, buffer, &len)) {
		printf("[Server] Failed to serialize message\n");
		return false;
	}

	if (!socket_send_all(ctx->clients[client_idx].socket_fd, buffer, len)) {
		printf("[Server] Send failed to client %d\n", client_idx);
		ctx->clients[client_idx].connected = false;
		return false;
	}

	return true;
}

bool server_recv_message(ServerContext *ctx, int client_idx, NetworkMessage *msg) {
	if (client_idx < 0 || client_idx >= 2) return false;
	if (!ctx->clients[client_idx].connected) return false;

	char buffer[MAX_MESSAGE_LEN];
	if (!socket_recv_all(ctx->clients[client_idx].socket_fd, buffer, MAX_MESSAGE_LEN)) {
		printf("[Server] Recv failed from client %d\n", client_idx);
		ctx->clients[client_idx].connected = false;
		return false;
	}

	if (!message_deserialize(buffer, MAX_MESSAGE_LEN, msg)) {
		printf("[Server] Deserialize failed\n");
		return false;
	}

	ctx->clients[client_idx].last_heartbeat = time(NULL);
	return true;
}

void server_cleanup(ServerContext *ctx) {
	for (int i = 0; i < 2; i++) {
		if (ctx->clients[i].connected) {
			close(ctx->clients[i].socket_fd);
		}
	}
	close(ctx->server_socket);
}

/* ========== Client Functions ========== */

bool client_connect(ClientContext *ctx, const char *ip, int port) {
	memset(ctx, 0, sizeof(ClientContext));
	strncpy(ctx->server_ip, ip, sizeof(ctx->server_ip) - 1);
	ctx->server_port = port;

	ctx->socket_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (ctx->socket_fd < 0) {
		perror("socket");
		return false;
	}

	struct sockaddr_in addr;
	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);

	if (inet_pton(AF_INET, ip, &addr.sin_addr) <= 0) {
		fprintf(stderr, "Invalid IP address: %s\n", ip);
		close(ctx->socket_fd);
		return false;
	}

	if (connect(ctx->socket_fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
		perror("connect");
		close(ctx->socket_fd);
		return false;
	}

	ctx->connected = true;
	ctx->last_heartbeat = time(NULL);
	printf("[Client] Connected to %s:%d\n", ip, port);
	return true;
}

bool client_send_message(ClientContext *ctx, const NetworkMessage *msg) {
	if (!ctx->connected) return false;

	char buffer[MAX_MESSAGE_LEN];
	int len = 0;
	if (!message_serialize(msg, buffer, &len)) {
		printf("[Client] Failed to serialize message\n");
		return false;
	}

	if (!socket_send_all(ctx->socket_fd, buffer, len)) {
		printf("[Client] Send failed\n");
		ctx->connected = false;
		return false;
	}

	return true;
}

bool client_recv_message(ClientContext *ctx, NetworkMessage *msg) {
	if (!ctx->connected) return false;

	char buffer[MAX_MESSAGE_LEN];
	if (!socket_recv_all(ctx->socket_fd, buffer, MAX_MESSAGE_LEN)) {
		printf("[Client] Recv failed\n");
		ctx->connected = false;
		return false;
	}

	if (!message_deserialize(buffer, MAX_MESSAGE_LEN, msg)) {
		printf("[Client] Deserialize failed\n");
		return false;
	}

	ctx->last_heartbeat = time(NULL);
	return true;
}

void client_disconnect(ClientContext *ctx) {
	if (ctx->connected) {
		close(ctx->socket_fd);
		ctx->connected = false;
	}
}

/* ========== Message Functions ========== */

void message_init(NetworkMessage *msg, MessageType type, int player_id) {
	memset(msg, 0, sizeof(NetworkMessage));
	msg->type = type;
	msg->player_id = player_id;
	msg->timestamp = (int)time(NULL);
}

bool message_serialize(const NetworkMessage *msg, char *buffer, int *len) {
	if (!buffer || !len) return false;

	/* Simple format: type(1)|player_id(4)|timestamp(4)|payload_len(4)|payload */
	int offset = 0;

	/* Type (1 byte) */
	buffer[offset++] = (char)msg->type;

	/* Player ID (4 bytes, network order) */
	int pid = htonl(msg->player_id);
	memcpy(&buffer[offset], &pid, 4);
	offset += 4;

	/* Timestamp (4 bytes, network order) */
	int ts = htonl(msg->timestamp);
	memcpy(&buffer[offset], &ts, 4);
	offset += 4;

	/* Payload length (4 bytes, network order) */
	int plen = htonl(strlen(msg->payload));
	memcpy(&buffer[offset], &plen, 4);
	offset += 4;

	/* Payload */
	int payload_len = strlen(msg->payload);
	if (payload_len > 0) {
		memcpy(&buffer[offset], msg->payload, payload_len);
		offset += payload_len;
	}

	*len = offset;
	return true;
}

bool message_deserialize(const char *buffer, int len, NetworkMessage *msg) {
	if (!buffer || len < 17) return false; /* Minimum: 1+4+4+4=13 + at least payload_len */

	memset(msg, 0, sizeof(NetworkMessage));
	int offset = 0;

	/* Type */
	msg->type = (MessageType)buffer[offset++];

	/* Player ID */
	int pid;
	memcpy(&pid, &buffer[offset], 4);
	msg->player_id = ntohl(pid);
	offset += 4;

	/* Timestamp */
	int ts;
	memcpy(&ts, &buffer[offset], 4);
	msg->timestamp = ntohl(ts);
	offset += 4;

	/* Payload length */
	int plen;
	memcpy(&plen, &buffer[offset], 4);
	plen = ntohl(plen);
	offset += 4;

	/* Payload */
	if (plen > 0 && offset + plen <= len) {
		memcpy(msg->payload, &buffer[offset], plen);
		msg->payload[plen] = '\0';
	}

	return true;
}

const char *message_type_str(MessageType type) {
	switch (type) {
	case MSG_CONNECT: return "CONNECT";
	case MSG_CONNECTION_ACK: return "CONNECTION_ACK";
	case MSG_CONNECTION_NAK: return "CONNECTION_NAK";
	case MSG_GAME_START: return "GAME_START";
	case MSG_YOUR_TURN: return "YOUR_TURN";
	case MSG_ROLL: return "ROLL";
	case MSG_MOVE_RESULT: return "MOVE_RESULT";
	case MSG_GAME_STATE: return "GAME_STATE";
	case MSG_WAITING: return "WAITING";
	case MSG_GAME_OVER: return "GAME_OVER";
	case MSG_HEARTBEAT: return "HEARTBEAT";
	case MSG_ERROR: return "ERROR";
	case MSG_DISCONNECT: return "DISCONNECT";
	case MSG_RECONNECT: return "RECONNECT";
	case MSG_FULL_SYNC: return "FULL_SYNC";
	default: return "UNKNOWN";
	}
}

/* ========== Utility Functions ========== */

bool validate_roll(int value) {
	return value >= 1 && value <= 6;
}

bool socket_has_data(int socket_fd, int timeout_sec) {
	fd_set readfds;
	FD_ZERO(&readfds);
	FD_SET(socket_fd, &readfds);

	struct timeval tv;
	tv.tv_sec = timeout_sec;
	tv.tv_usec = 0;

	int ret = select(socket_fd + 1, &readfds, NULL, NULL, &tv);
	return ret > 0;
}

void socket_set_timeout(int socket_fd, int timeout_sec) {
	struct timeval tv;
	tv.tv_sec = timeout_sec;
	tv.tv_usec = 0;
	setsockopt(socket_fd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
	setsockopt(socket_fd, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(tv));
}

bool socket_send_all(int socket_fd, const char *buffer, int len) {
	int sent = 0;
	while (sent < len) {
		int n = send(socket_fd, &buffer[sent], len - sent, 0);
		if (n < 0) {
			perror("send");
			return false;
		}
		if (n == 0) {
			return false; /* Connection closed */
		}
		sent += n;
	}
	return true;
}

bool socket_recv_all(int socket_fd, char *buffer, int len) {
	int recv_len = recv(socket_fd, buffer, len, 0);
	if (recv_len < 0) {
		perror("recv");
		return false;
	}
	if (recv_len == 0) {
		return false; /* Connection closed */
	}
	return true;
}
