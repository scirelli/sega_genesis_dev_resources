#!/usr/bin/env bash
# Notes:
# - The trailing / on SOURCE_DIR matters — it copies the contents of the folder rather than the folder itself
# - --delete removes files in the destination that no longer exist in the source (keeps them in sync); remove it if you want to only add/update without deleting
# - git add stages the synced files so they're included in the commit you're about to make
SOURCE_DIR="$HOME/VimWiki/projects/sega_genesis/"
DEST_DIR="Documentation/wiki/"
                                                                                                                                
rsync -a --delete "$SOURCE_DIR" "$DEST_DIR"
git add "$DEST_DIR"
