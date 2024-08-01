# ppl-process-scores
Data processor for custom ppl leaderboards
## Build
`g++ -std=c++17 -O2 -o process_scores process_scores.cpp`
## Run
`./process_scores`  
`./process_scores folder`  
`./process_scores folder/`  
All files are inputed and outputed in local or specified directory.
## Input
* `score_data.csv` - all scores.
* `level_data.csv` - level uuid/name mapping.
* `account_data.csv` - account uuid/name mapping.
* `monthly_leaderboard_levels.txt` - levels, which will be used in monthly leaderboard. No quotation marks, every new level name starts from next line.
## Output
* `monthly_leaderboard.csv` - monthly leaderboard.
* `era2_leaderboard.csv` - standard era2 leaderboard. **TBD**
* `era2_leaderboard_1p.csv` - 1p leaderboard with era2 formula. **TBD**
* `era2_leaderboard_2p.csv` - 2p leaderboard with era2 formula. **TBD**
