# Phase 10: Network Multiplayer — Enhanced Architecture

## 🏗️ Enhanced Client-Server Model

```
╔════════════════════════════════════════════════════════════════════════════╗
║                         NETWORK MULTIPLAYER FLOW                           ║
╚════════════════════════════════════════════════════════════════════════════╝

SERVER (Host Machine)                      CLIENT (Remote Machine)
═════════════════════════════════════════  ════════════════════════════════

1. START: start_server(port 5000)          1. START: connect_client(ip, port)
   Listen on port 5000
   
   msg: server_init | port:5000 ──────────────────────────────────────
                                                        ↓
                                              Try connect... retries: 3
                                              
2. HANDSHAKE:                              2. HANDSHAKE:
   Wait for 2 clients                         Send: MSG_CONNECT | player_name
   Assign Player 1, Player 2                  ↑_________________________________|
   
   msg: MSG_CONNECTION_ACK ──────────────────────────────────────────
        | player_id: 1
        | opponent_name: "Bob"
                                              Recv: Who am I? Board setup
                                              
3. START GAME:                             3. DISPLAY SETUP:
   game_init(2 players)                       Show: "You are Player 1"
   Send: MSG_GAME_START                       "Opponent: Bob"
        | board_state
        | player_positions
        | initial_turn: PLAYER_1
        
        ────────────────────────────────────→ Render board
                                              
4. GAME LOOP:                              4. GAME LOOP:
                                              
   Turn = PLAYER_1 (Local):                   Turn = PLAYER_1 (Me):
   Execute local move immediately             Wait for user input
   Send: MSG_STATE_UPDATE                     [ENTER] to roll dice
        | turn_data
        | new_positions
        | winner_status
                                              Send: MSG_ROLL | value
        ←──────────────────────────────────── 
                                              
   Recv: MSG_ROLL:                            
   • Validate roll (1-6)
   • Execute move
   • Update board
   • Check for snake/ladder
   • Check for win
   • Switch turn
   
   Send: MSG_STATE_UPDATE                     
        | updated_board
        | current_turn: PLAYER_2
        | game_phase: ACTIVE/WIN
        
        ────────────────────────────────────→
                                              Recv: MSG_STATE_UPDATE
                                              Update local board
                                              Display: "Bob's turn..."
                                              
   Turn = PLAYER_2 (Remote):
   Send: MSG_WAITING                          
        | message: "Waiting for opponent..."
        
        ────────────────────────────────────→
                                              Display: "Your turn!"
                                              [ENTER] to roll...
                                              Send: MSG_ROLL | value
        ←──────────────────────────────────── 
                                              
   [Execute move on server]
   [Repeat until winner]
   
5. GAME END:                               5. GAME END:
   Send: MSG_GAME_OVER                        Recv: MSG_GAME_OVER
        | winner: PLAYER_2                    Display: "You lost! Bob won!"
        | final_stats                         Option: Play again? Exit
        | analytics_data
        
6. DISCONNECT/ERROR:                      6. DISCONNECT/ERROR:
   MSG_DISCONNECT                            Graceful exit or reconnect
   Close socket                              Retry connection
   Cleanup resources                        Timeout after 30s
```

---

## 📨 Message Protocol (Upgraded)

### **Message Types with Full Details**

```c
/* Connection Phase */
MSG_CONNECT         → { name: "Player1", version: 1 }
MSG_CONNECTION_ACK  ← { player_id: 1, opponent: "Player2", board_state: "..." }
MSG_CONNECTION_NAK  ← { reason: "Game full" / "Invalid version" }

/* Game State Phase */
MSG_GAME_START      ← { board: [...], positions: [0,0], turn: 0 }
MSG_YOUR_TURN       ← { timeout_sec: 30 }
MSG_ROLL            → { value: 4, timestamp: 12345 }
MSG_MOVE_RESULT     ← { new_pos: 25, event: "LADDER", current_turn: 1 }
MSG_GAME_STATE      ← { 
                      board: [...], 
                      p1_pos: 25, 
                      p2_pos: 10,
                      turn: 1,
                      dice_roll: 4,
                      move_num: 5,
                      phase: "ACTIVE"
                    }
MSG_WAITING         ← { message: "Opponent turn..." }

/* Game End Phase */
MSG_GAME_OVER       ← { winner_id: 1, final_stats: {...} }
MSG_REPLAY_AVAILABLE ← { can_watch: true }

/* Error Handling */
MSG_HEARTBEAT       ↔ { ping: timestamp } → { pong: timestamp }
MSG_ERROR           ← { code: 1001, reason: "Invalid move" }
MSG_DISCONNECT      ↔ { reason: "User quit" / "Timeout" }

/* Reconnection & Recovery */
MSG_RECONNECT       → { player_id: 1, session_id: "..." }
MSG_SYNC_REQUEST    → { last_move_id: 10 }
MSG_FULL_SYNC       ← { move_history: [...], current_state: {...} }
```

---

## 🔄 Enhanced State Synchronization

```
Server State                          Client State (Local Cache)
═════════════════════════════════════  ══════════════════════════════

⓵ Current Board                        ⓵ Predicted Board
   [Authoritative]                        [Optimistic UI]
   
⓶ Player Positions                     ⓶ Player Positions
   [Locked, validated]                    [Estimated until ACK]
   
⓷ Turn Counter                         ⓷ Turn Counter
   [Single source of truth]               [Updated on MSG_STATE]
   
⓸ Move History                         ⓸ Move History
   [Full replay log]                      [Live moves only]
   
⓹ Analytics                            ⓹ Analytics
   [Server-side only]                     [Display on game end]
   
⓺ Disconnection Flag                   ⓺ Reconnection Logic
   [Timeout after 30s]                    [Retry with backoff]
```

---

## 🛡️ Validation & Security

### **Server-side Validation (Anti-Cheating)**

```
Recv: MSG_ROLL { value: 4 }

Checks:
✓ Is it this player's turn?
✓ Is value in [1, 6]?
✓ Did they already roll this turn?
✓ Is game not paused/over?
✓ Timestamp reasonable? (within 5s of server)

If any check fails:
  Send: MSG_ERROR { code: 1001, reason: "Invalid move" }
  Don't execute move
  
If all pass:
  Execute move atomically
  Send: MSG_GAME_STATE (confirmed state)
```

### **Client-side Validation (UX)**

```
Before sending MSG_ROLL:
✓ Is game connected?
✓ Is display showing "Your turn"?
✓ Did user press [ENTER]?

Display feedback:
✓ "Sending roll..." (loading state)
✓ Disable input until ACK received
✓ Timeout warning if >5s
✓ Reconnect dialog if disconnected
```

---

## 🔌 Connection Management

### **Lifecycle**

```
STATE: DISCONNECTED
  ↓ connect_client(ip, port)
  
STATE: CONNECTING (timeout: 5s)
  ↓ [Success]
  
STATE: HANDSHAKE (exchange names/IDs)
  ↓ [Both handshakes complete]
  
STATE: WAITING_FOR_OPPONENT (timeout: 30s)
  ↓ [2nd player connects]
  
STATE: GAME_ACTIVE
  ↓ [Loop until game_over OR disconnect]
  
STATE: GAME_OVER
  ↓ offer_replay() OR quit()
  
STATE: DISCONNECTED (cleanup)
```

### **Reconnection Strategy**

```
if connection_lost {
  Retry 1: Wait 1s, reconnect
  Retry 2: Wait 2s, reconnect
  Retry 3: Wait 4s, reconnect
  Retry 4: Wait 8s, reconnect
  Failed: Show "Connection Lost" dialog
}

if reconnect_successful {
  Send: MSG_RECONNECT { player_id, session_id }
  Recv: MSG_FULL_SYNC { move_history[] }
  Rebuild board from history
  Resume game
}
```

---

## 📊 Data Flow: Detailed Move Execution

```
CLIENT SIDE                              NETWORK                  SERVER SIDE
═══════════════════════════════════════  ═════════════════════════  ═════════════════════════

Display: "Your turn!"

User presses [ENTER]
  │
  ├─ Validate: Is my turn? YES
  │
  ├─ Generate: roll = rand(1,6) ✓
  │
  ├─ Show: "Rolling dice..."
  │
  ├─ Send:
  │  ┌──────────────────────────┐
  │  │ MSG_ROLL                 │
  │  │ { value: 4,              │
  │  │   timestamp: 1685048125, │
  │  │   player_id: 1 }         │
  │  └──────────────────────────┘
  │     │
  │     ├──────────────────────────────→ Recv: MSG_ROLL
  │     │                                 │
  │     │                                 ├─ Validate:
  │     │                                 │   ✓ Player 1's turn?
  │     │                                 │   ✓ Value 1-6? (4 ✓)
  │     │                                 │   ✓ Timestamp fresh?
  │     │                                 │   ✓ Not already rolled?
  │     │                                 │
  │     │                                 ├─ Execute move:
  │     │                                 │   old_pos = 20
  │     │                                 │   new_pos = 20 + 4 = 24
  │     │                                 │   board_cell = LADDER
  │     │                                 │   resolved_pos = 38
  │     │                                 │
  │     │                                 ├─ Update server state:
  │     │                                 │   players[0].position = 38
  │     │                                 │   turn_number = 2
  │     │                                 │   current_turn = PLAYER_2
  │     │                                 │
  │     │                                 ├─ Send:
  │     │                                 │  ┌─────────────────────────┐
  │     │  ←────────────────────────────  │  │ MSG_GAME_STATE          │
  │     │                                 │  │ { board: [...],         │
  │     │  Recv: MSG_GAME_STATE           │  │   p1_pos: 38,           │
  │     │  { board: [...],                │  │   p2_pos: 10,           │
  │     │    p1_pos: 38,                  │  │   turn: 1,              │
  │     │    p2_pos: 10,                  │  │   event: "LADDER",      │
  │     │    turn: 1,                     │  │   move_num: 5 }         │
  │     │    event: "LADDER",             │  └─────────────────────────┘
  │     │    move_num: 5 }                │
  │     │                                 │
  ├─ Update local board:
  │  players[0].position = 38
  │  board[38] = PLAYER_1
  │  board[20] = EMPTY
  │
  ├─ Display:
  │  "You rolled 4!"
  │  "Ladder! Climbed to 38"
  │  "Waiting for opponent..."
  │
  └─ Disable input (wait for next turn)

                                          ├─ Check for win:
                                          │  No (38 ≠ 100)
                                          │
                                          ├─ Send:
                                          │  ┌──────────────────┐
                                          │  │ MSG_YOUR_TURN    │
                                          │  │ (to Player 2)    │
                                          │  └──────────────────┘
                                          │
                                          └─ Wait for Player 2's move...

                                                              Display: "Your turn!"
                                                              [ENTER] to roll...
```

---

## 🎯 Implementation Checklist

```
Phase 10 Implementation Tasks
════════════════════════════════════════════════════════════════

Network Foundation:
  ☐ include/network.h (message structs, socket utilities)
  ☐ src/network.c (send/recv/validate functions)
  ☐ Message parsing & serialization

Server:
  ☐ src/server.c (server loop, client management)
  ☐ Authoritative game loop
  ☐ Move validation
  ☐ State broadcasting
  ☐ src/main_server.c (entry point)

Client:
  ☐ src/client.c (UI, network sync)
  ☐ Optimistic rendering
  ☐ Error recovery
  ☐ Reconnection logic
  ☐ src/main_client.c (entry point)

Build & Testing:
  ☐ Update Makefile (2 binaries)
  ☐ Test locally (same machine)
  ☐ Test remotely (different machines)
  ☐ Test network failures
  ☐ Update README.md

Deployment:
  ☐ Author section in README
  ☐ GitHub commit & push
```

---

## 📝 Usage Example (After Implementation)

```bash
# Terminal 1 (Server)
$ ./snake-ladder-server 5000
[Server] Listening on port 5000...
[Server] Player 1 connected: Alice
[Server] Player 2 connected: Bob
[Server] Game started!
[Server] Turn: Alice
[Server] Waiting for Alice's move...
[Server] Alice rolled 4 → moved to 4
[Server] Turn: Bob
...

# Terminal 2 (Client 1 - Alice)
$ ./snake-ladder-client localhost 5000
[Client] Connecting to localhost:5000...
[Client] Connected! You are Player 1 (Alice)
[Client] Opponent: Bob
[Board displays]
Alice - Cell: 0
Bob   - Cell: 0
Your turn! [ENTER] to roll:
[User presses ENTER]
You rolled 4!
Waiting for Bob...

# Terminal 3 (Client 2 - Bob)
$ ./snake-ladder-client localhost 5000
[Client] Connecting to localhost:5000...
[Client] Connected! You are Player 2 (Bob)
[Client] Opponent: Alice
[Board displays]
Alice - Cell: 4
Bob   - Cell: 0
Waiting for Alice...
Alice rolled 4 and moved to 4
Your turn! [ENTER] to roll:
...
```

---

## 🔐 Key Improvements Over Basic Architecture

| Aspect | Basic | **Enhanced** |
|--------|-------|------------|
| **Message Types** | 3 | **7+ with errors** |
| **Validation** | None | **Server-side + client-side** |
| **Error Handling** | None | **Retry, timeout, reconnect** |
| **State Sync** | Full state only | **Differential updates** |
| **Disconnection** | Crash | **Graceful recovery** |
| **Cheating Prevention** | Server trusts client | **Server validates every move** |
| **Network Latency** | No optimization | **Optimistic UI updates** |
| **Analytics** | Server only | **Full replay persistence** |

---

## ✅ This Architecture Supports

✓ Casual 1v1 games
✓ Network resilience (reconnection)
✓ Cheat prevention (server validation)
✓ Smooth UI (optimistic updates)
✓ Production-grade error handling
✓ Future scaling (multi-room games)
