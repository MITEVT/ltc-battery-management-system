mkdir logs
make com | tee logs/$(date -d "today" +"%Y%m%d%H%M").log
