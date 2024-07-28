# ppl-process-scores
Data processor for custom ppl leaderboards
## Build
`g++ -std=c++17 -o process_scores process_scores.cpp`
## Run
`./process_scores`  
`./process_scores folder`  
`./process_scores folder/`  
All files are inputed and outputed in local or specified directory.
## Input
* `score.csv` - all scores.
* `monthly_leaderboard_levels.txt` - levels, which will be used in monthly leaderboard. No quotation marks, every new level name starts from next line.
## Output
* `monthly_leaderboard.csv` - monthly leaderboard.
* `era2_leaderboard.csv` - standard era2 leaderboard.
* `era2_leaderboard_1p.csv` - 1p leaderboard with era2 formula.
* `era2_leaderboard_2p.csv` - 2p leaderboard with era2 formula.
