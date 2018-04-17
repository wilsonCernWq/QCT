#!/bin/bash
cat > hostname.$PMI_RANK.txt <<EOF
$(hostname)
EOF
