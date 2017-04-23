mkdir logs
make com | tee logs/$(date -d "today" +"%Y%m%d%H%M").log
python scripts/parse_log.py logs/$(date -d "today" +"%Y%m%d%H%M").log logs/$(date -d "today" +"%Y%m%d%H%M")_voltages.csv logs/$(date -d "today" +"%Y%m%d%H%M")_temps.csv
