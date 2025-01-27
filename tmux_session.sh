#!/usr/bin/env bash

SESSION_NAME="paca_engine"
export PATH=$PWD/scripts:$PATH

# Check if the session already exists
if tmux has-session -t $SESSION_NAME 2>/dev/null; then
    echo "Session $SESSION_NAME already exists. Attaching to it."
    tmux attach-session -t $SESSION_NAME
else
    tmux new-session -d -s $SESSION_NAME #$DEV_SHELL_CMD

    tmux send-keys -t "${SESSION_NAME}:0.0" "nvim" C-m

    tmux new-window
    tmux send-keys -t "${SESSION_NAME}:1.0" "cd build" C-m
    
    #tmux new-window
    #tmux send-keys -t "${SESSION_NAME}:2.0" "lazygit" C-m

    tmux next-window

    # Attach to the created session
    tmux attach-session -t $SESSION_NAME
fi
