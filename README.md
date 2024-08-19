# ppl-process-scores
Data processor for custom ppl leaderboards
## Build
`g++ -std=c++17 -O2 -o process_scores process_scores.cpp`
## Run
`./process_scores`  
`./process_scores -input_path=/input`  
`./process_scores -output_path=/output`  
`./process_scores -input_path=/input/ -output_path=/output/`  
All files are inputed and outputed in local or respective specified directory.
## Input
* `score_data.csv` - all scores.
* `level_data.csv` - level uuid/name mapping.
* `account_data.csv` - account uuid/name mapping.
* `monthly_leaderboard_levels.txt` - levels, which will be used in monthly leaderboard. No quotation marks, every new level name starts from next line.
* `banned_monthly_leaderboard_levels.txt` - levels, which won't be used while randomly generating level pool for monthly leaderboard.
## Output
* `levels.txt` - list of all level names.
* `monthly_leaderboard_pool.txt` - randomly generated list of levels for monthly leaderboard initial pool.
Data in each row in next files is stored as:  
`(string)account_uuid,(string)"account_name",(string)country,(int)score,(int)WR_amount,(double)average_score\n`  
* `monthly_leaderboard.csv` - monthly leaderboard.
* `era_leaderboard_1p.csv` - 1p leaderboard with era2 formula.
* `era_speedrun_leaderboard_1p.csv` - 1p speedrun leaderboard with era2 formula.
* `era_leaderboard_2p.csv` - 2p leaderboard with era2 formula.
* `era_speedrun_leaderboard_2p.csv` - 2p speedrun leaderboard with era2 formula.
* `era_leaderboard_2p_fixed.csv` - 2p leaderboard with era2 formula. Only best scores are accounted for in this leaderboard.
* `era_speedrun_leaderboard_2p_fixed.csv` - 2p speedrun leaderboard with era2 formula. Only best scores are accounted for in this leaderboard.
