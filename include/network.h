#ifndef NETWORK_H
#define NETWORK_H

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdbool.h>
#include <time.h>

/* Network constants */
#define DEFAULT_PORT 5000
#define MAX_MESSAGE_LEN 4096
#define SOCKET_TIMEOUT_SEC 30
#define HEARTBEAT_INTERVAL_SEC 5

/* Message types */
typedef enum {
	MSG_CONNECT = 1,
	MSG_CONNECTION_ACK = 2,
	MSG_CONNECTION_NAK = 3,
	MSG_GAME_START = 4,
	MSG_YOUR_TURN = 5,
	MSG_ROLL = 6,
	MSG_MOVE_RESULT = 7,
	MSG_GAME_STATE = 8,
	MSG_WAITING = 9,
	MSG_GAME_OVER = 10,
	MSG_HEARTBEAT = 11,
	MSG_ERROR = 12,
	MSG_DISCONNECT = 13,
	MSG_RECONNECT = 14,
	MSG_FULL_SYNC = 15,
} MessageType;

/* Note: GamePhase is defined in game.h to avoid conflicts */

/* Network message structure */
typedef struct {
	MessageType type;
	int player_id;
	int timestamp;
	char payload[MAX_MESSAGE_LEN];
} NetworkMessage;

/* Connection metadata */
typedef struct {
	int socket_fd;
	char player_name[64];
	int player_id;
	struct sockaddr_in addr;
	time_t last_heartbeat;
	bool connected;
} ClientConnection;

/* Server context */
typedef struct {
	int server_socket;
	int port;
	ClientConnection clients[2];
	int num_connected;
	time_t start_time;
} ServerContext;

/* Client context */
typedef struct {
	int socket_fd;
	char server_ip[256];
	int server_port;
	int player_id;
	char opponent_name[64];
	bool connected;
	time_t last_heartbeat;
} ClientContext;

/* ========== Socket Functions ========== */

/* Server functions */
bool server_init(ServerContext *ctx, int port);
bool server_accept_clients(ServerContext *ctx);
bool server_send_message(ServerContext *ctx, int client_idx, const NetworkMessage *msg);
bool server_recv_message(ServerContext *ctx, int client_idx, NetworkMessage *msg);
void server_cleanup(ServerContext *ctx);

/* Client functions */
bool client_connect(ClientContext *ctx, const char *ip, int port);
bool client_send_message(ClientContext *ctx, const NetworkMessage *msg);
bool client_recv_message(ClientContext *ctx, NetworkMessage *msg);
void client_disconnect(ClientContext *ctx);

/* Message functions */
void message_init(NetworkMessage *msg, MessageType type, int player_id);
bool message_serialize(const NetworkMessage *msg, char *buffer, int *len);
bool message_deserialize(const char *buffer, int len, NetworkMessage *msg);
const char *message_type_str(MessageType type);

/* ========== Utility Functions ========== */
bool validate_roll(int value);
bool socket_has_data(int socket_fd, int timeout_sec);
void socket_set_timeout(int socket_fd, int timeout_sec);
bool socket_send_all(int socket_fd, const char *buffer, int len);
bool socket_recv_all(int socket_fd, char *buffer, int len);

#endif
