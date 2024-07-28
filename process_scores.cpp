#include <iostream>
#include <cmath>
#include <filesystem>
#include <vector>
#include <fstream>
#include <regex>

struct Score1p {
  unsigned long int timestamp;
  int era;
  int score;
  std::string name;
  std::string level_name;
  std::string country;
  std::string platform;

  Score1p(unsigned long int _timestamp, int _era, int _score, std::string& _name, std::string& _level_name, std::string& _country, std::string& _platform) {
    timestamp = _timestamp;
    era = _era;
    score = _score;
    name = _name;
    level_name = _level_name;
    country = _country;
    platform = _platform;
  }

  void print() {
    std::cout << timestamp
      << " " << era
      << " " << score
      << " \"" << name
      << "\" \"" << level_name
      << "\" \"" << country
      << "\" \"" << platform << "\"\n";
  }
};

struct Score2p {
  unsigned long int timestamp;
  int era;
  int score;
  std::string name1;
  std::string name2;
  std::string level_name;
  std::string country;

  Score2p(unsigned long int _timestamp, int _era, int _score, std::string& _name1, std::string& _name2, std::string& _level_name, std::string& _country) {
    timestamp = _timestamp;
    era = _era;
    score = _score;
    name1 = _name1;
    name2 = _name2;
    level_name = _level_name;
    country = _country;
  }

  void print() {
    std::cout << timestamp
      << " " << era
      << " " << score
      << " \"" << name1
      << "\" \"" << name2
      << "\" \"" << level_name
      << "\" \"" << country << "\"\n";
  }
};

struct Score {
  unsigned long int timestamp;
  int era;
  int mode; // 0 - singleplayer, 1 - multiplayer
  int score;
  std::string name1;
  std::string name2; // empty string, if mode == 0
  std::string level_name;
  std::string country;
  std::string platform;

  void print() {
    std::cout << timestamp
      << " " << era
      << " " << mode
      << " " << score
      << " \"" << name1
      << "\" \"" << name2
      << "\" \"" << level_name
      << "\" \"" << country
      << "\" \"" << platform << "\"\n";
  }

  Score1p to_1p() {
    return Score1p(timestamp, era, score, name1, level_name, country, platform);
  }

  Score2p to_2p() {
    return Score2p(timestamp, era, score, name1, name2, level_name, country);
  }

  bool is_1p() {
    return mode == 0;
  }
};

struct Leaderboard1p {
  std::string level_name;
  std::vector<Score1p> scores;

  Leaderboard1p() {}

  Leaderboard1p(std::string& name) {
    level_name = name;
  }

  int index_of_score_by(std::string& name) {
    for (int i = 0; i < scores.size(); ++i)
      if (scores[i].name == name)
        return i;
    return -1;
  }

  void print() {
    std::cout << level_name << '\n';
    for (int i = 0; i < scores.size(); ++i) {
      std::cout << i + 1 << ". ";
      scores[i].print();
    }
    std::cout << std::endl;
  }
};

struct Leaderboard2p {
  std::string level_name;
  std::vector<Score2p> scores;

  Leaderboard2p() {}

  Leaderboard2p(std::string& name) {
    level_name = name;
  }

  int index_of_score_by(std::string& name1, std::string& name2) {
    for (int i = 0; i < scores.size(); ++i)
      if (scores[i].name1 == name1 && scores[i].name2 == name2 || scores[i].name1 == name2 && scores[i].name2 == name1)
        return i;
    return -1;
  }

  void print() {
    std::cout << level_name << " 2p\n";
    for (int i = 0; i < scores.size(); ++i) {
      std::cout << i + 1 << ". ";
      scores[i].print();
    }
    std::cout << std::endl;
  }
};

struct CustomScore {
  double score;
  std::string name;
  std::string country;
  std::string platform;

  CustomScore(Score1p score) {
    name = score.name;
    country = score.country;
    platform = score.platform;
  }

  void print() {
    std::cout << score
      << " \"" << name
      << "\" \"" << country
      << "\" \"" << platform << "\"\n";
  }
};

struct CustomLeaderboard {
  std::vector<CustomScore> scores;

  int index_of_score_by(std::string& name) {
    for (int i = 0; i < scores.size(); ++i)
      if (scores[i].name == name)
        return i;
    return -1;
  }

  void print() {
    std::cout << "Custom leaderboard\n";
    for (int i = 0; i < scores.size(); ++i) {
      std::cout << i + 1 << ". ";
      scores[i].print();
    }
    std::cout << std::endl;
  }
};

static std::string scores_file = "score.csv";
static std::string monthly_leaderboard_levels_file = "monthly_leaderboard_levels.txt";
static std::string monthly_leaderboard_output_file = "monthly_leaderboard.csv";
static std::string era2_leaderboard_output_file = "era2_leaderboard.csv";
static std::string era2_leaderboard_output_file_1p = "era2_leaderboard_1p.csv";
static std::string era2_leaderboard_output_file_2p = "era2_leaderboard_2p.csv";

static std::vector<std::string> monthly_leaderboard_levels;
static std::vector<Score> scores;
static std::vector<Score1p> scores_1p;
static std::vector<Score2p> scores_2p;
static std::vector<Leaderboard1p> level_leaderboards_1p;
static std::vector<Leaderboard2p> level_leaderboards_2p;
static CustomLeaderboard monthly_leaderboard;
static CustomLeaderboard era2_leaderboard;
static CustomLeaderboard era2_leaderboard_1p;
static CustomLeaderboard era2_leaderboard_2p;

int vector_index_of(std::vector<std::string>& vec, std::string& str) {
  for (int i = 0; i < vec.size(); ++i)
    if (vec[i] == str)
      return i;
  return -1;
}

int leaderboards_1p_index_of_level(std::string& level_name) {
  for (int i = 0; i < level_leaderboards_1p.size(); ++i)
    if (level_leaderboards_1p[i].level_name == level_name)
      return i;
  return -1;
}

int leaderboards_2p_index_of_level(std::string& level_name) {
  for (int i = 0; i < level_leaderboards_2p.size(); ++i)
    if (level_leaderboards_2p[i].level_name == level_name)
      return i;
  return -1;
}

std::vector<std::string> split_line(std::string& line) {
  std::vector<std::string> strings;
  std::string str = "";
  bool in_quotes = false;
  for (char c : line) {
    if (c == '\"')
      in_quotes = !in_quotes;
    else if (c == ',' && !in_quotes) {
      str.erase(remove(str.begin(), str.end(), '\"'), str.end());
      strings.push_back(str);
      str = "";
    }
    else
      str += c;
  }
  str.erase(remove(str.begin(), str.end(), '\"'), str.end());
  strings.push_back(str);
  return strings;
}

Score process_line(std::string& line) {
  std::vector<std::string> strings = split_line(line);
  Score score;
  score.timestamp = std::stoul(strings[0]);
  score.era = std::stoi(strings[1]);
  score.name1 = strings[2];
  score.name2 = strings[3];
  score.level_name = strings[4];
  score.score = std::stoi(strings[5]);
  score.country = strings[6];
  score.platform = strings[7];
  score.mode = std::stoi(strings[8]);
  return score;
}

void fix_line(std::string& line) {
  line.erase(std::remove(line.begin(), line.end(), '\r'), line.end());
}

int load_scores() {
  if (std::filesystem::exists(scores_file))
    std::cout << "Processing data." << std::endl;
  else {
    std::cout << "Scores file doesn't exist." << std::endl;
    return 1;
  }

  std::ifstream scores_fstream(scores_file);
  if (!scores_fstream.is_open()) {
    std::cout << "Failed to open scores file." << std::endl;
    return 1;
  }

  std::string line;
  while (std::getline(scores_fstream, line)) {
    fix_line(line);
    scores.push_back(process_line(line));
  }
  scores_fstream.close();
  return 0;
}

int load_monthly_leaderboard_levels() {
  if (std::filesystem::exists(monthly_leaderboard_levels_file))
    std::cout << "Loading monthly leaderboard levels." << std::endl;
  else {
    std::cout << "Monthly leaderboard levels file doesn't exist." << std::endl;
    return 1;
  }
  std::ifstream monthly_leaderboard_levels_fstream(monthly_leaderboard_levels_file);
  if (!monthly_leaderboard_levels_fstream.is_open()) {
    std::cout << "Failed to open monthly leaderboard levels file." << std::endl;
    return 1;
  }

  std::string line;
  while (std::getline(monthly_leaderboard_levels_fstream, line)) {
    fix_line(line);
    monthly_leaderboard_levels.push_back(line);
  }
  monthly_leaderboard_levels_fstream.close();
  return 0;
}

Leaderboard1p create_leaderboard_1p(std::string& level_name) {
  Leaderboard1p leaderboard(level_name);
  for (Score1p& score : scores_1p)
    if (score.level_name == level_name) {
      int index = leaderboard.index_of_score_by(score.name);
      if (index == -1)
        leaderboard.scores.push_back(score);
      else
        leaderboard.scores[index] = score;
    }
  return leaderboard;
}

void process_scores() {
  for (Score& score : scores)
    if (score.is_1p())
      scores_1p.push_back(score.to_1p());
    else
      scores_2p.push_back(score.to_2p());
  scores.clear();

  for (Score1p& score : scores_1p) {
    int level_index = leaderboards_1p_index_of_level(score.level_name);
    if (level_index == -1) {
      Leaderboard1p leaderboard(score.level_name);
      leaderboard.scores.push_back(score);
      level_leaderboards_1p.push_back(leaderboard);
    }
    else {
      int score_index = level_leaderboards_1p[level_index].index_of_score_by(score.name);
      if (score_index == -1)
        level_leaderboards_1p[level_index].scores.push_back(score);
      else
        level_leaderboards_1p[level_index].scores[score_index] = score;
    }
  }
  for (Leaderboard1p& leaderboard : level_leaderboards_1p)
    std::sort(leaderboard.scores.begin(), leaderboard.scores.end(), [](const Score1p& a, const Score1p& b) {
    return a.score > b.score;
      });

  for (Score2p& score : scores_2p) {
    int level_index = leaderboards_2p_index_of_level(score.level_name);
    if (level_index == -1) {
      Leaderboard2p leaderboard(score.level_name);
      leaderboard.scores.push_back(score);
      level_leaderboards_2p.push_back(leaderboard);
    }
    else {
      int score_index = level_leaderboards_2p[level_index].index_of_score_by(score.name1, score.name2);
      if (score_index == -1)
        level_leaderboards_2p[level_index].scores.push_back(score);
      else if (score.score > level_leaderboards_2p[level_index].scores[score_index].score)
        level_leaderboards_2p[level_index].scores[score_index] = score;
    }
  }
  for (Leaderboard2p& leaderboard : level_leaderboards_2p)
    std::sort(leaderboard.scores.begin(), leaderboard.scores.end(), [](const Score2p& a, const Score2p& b) {
    return a.score > b.score;
      });
}

int get_monthly_leaderboard_score(int i) {
  if (i < 3)
    return 2500 - i * 250;
  else if (i < 10)
    return 2125 - i * 125;
  else if (i < 25)
    return 1450 - i * 50;
  return 0;
}

const double __era2_r = 100.f * sqrt(2.f);
double get_era2_leaderboard_score(int rank, int player_amount) {
  return __era2_r * pow(player_amount, 1.f / 6.f) / sqrt(rank);
}

void create_monthly_leaderboard() {
  for (std::string& monthly_level_name : monthly_leaderboard_levels) {
    int level_index = leaderboards_1p_index_of_level(monthly_level_name);
    Leaderboard1p level_leaderboard;
    if (level_index == -1) {
      std::cout << "Incorrect level name was provided during monthly leaderboard calculation: \"" << monthly_level_name << "\".\n";
      continue;
    } else
      level_leaderboard = level_leaderboards_1p[level_index];
    for (int i = 0; i < 25 && i < level_leaderboard.scores.size(); ++i) {
      int score_index = monthly_leaderboard.index_of_score_by(level_leaderboard.scores[i].name);
      if (score_index == -1) {
        CustomScore score(level_leaderboard.scores[i]);
        score.score = get_monthly_leaderboard_score(i);
        monthly_leaderboard.scores.push_back(score);
      }
      else
        monthly_leaderboard.scores[score_index].score += get_monthly_leaderboard_score(i);
    }
  }
  std::sort(monthly_leaderboard.scores.begin(), monthly_leaderboard.scores.end(), [](const CustomScore& a, const CustomScore& b) {
    return a.score > b.score;
    });
}

int extract_monthly_leaderboard() {
  std::cout << "Extracting monthly leaderboard." << std::endl;
  std::ofstream monthly_leaderboard_fstream(monthly_leaderboard_output_file);
  if (!monthly_leaderboard_fstream.is_open()) {
    std::cout << "Failed to open monthly leaderboard output file." << std::endl;
    return 1;
  }

  for (CustomScore& score : monthly_leaderboard.scores)
    monthly_leaderboard_fstream << "\"" << score.name << "\"," << score.score << "," << score.country << "," << score.platform << "\n";
  monthly_leaderboard_fstream.close();
  return 0;
}

void create_era2_leaderboards() {
  for (Leaderboard1p& level_leaderboard : level_leaderboards_1p) {
    for (int i = 0; i < level_leaderboard.scores.size(); ++i) {
      int score_index = era2_leaderboard_1p.index_of_score_by(level_leaderboard.scores[i].name);
      if (score_index == -1) {
        CustomScore score(level_leaderboard.scores[i]);
        score.score = get_era2_leaderboard_score(i + 1, level_leaderboard.scores.size());
        era2_leaderboard_1p.scores.push_back(score);
      }
      else
        era2_leaderboard_1p.scores[score_index].score += get_era2_leaderboard_score(i + 1, level_leaderboard.scores.size());
    }
  }
  std::sort(era2_leaderboard_1p.scores.begin(), era2_leaderboard_2p.scores.end(), [](const CustomScore& a, const CustomScore& b) {
    return a.score > b.score;
    });
}

int extract_era2_leaderboards() {
  std::cout << "Extracting era2 leaderboard." << std::endl;
  std::ofstream era2_leaderboard_fstream(era2_leaderboard_output_file);
  if (!era2_leaderboard_fstream.is_open()) {
    std::cout << "Failed to open era2 leaderboard output file." << std::endl;
    return 1;
  }

  era2_leaderboard_fstream << std::fixed << std::setprecision(4);
  for (CustomScore& score : era2_leaderboard.scores)
    era2_leaderboard_fstream << "\"" << score.name << "\"," << score.score << "," << score.country << "," << score.platform << "\n";
  era2_leaderboard_fstream.close();
  return 0;
}

int main(int argc, char* argv[]) {

  bool testing = false;

  if (!testing)
    if (argc > 1) {
      std::cout << "Path was provided, using it as a root directory." << std::endl;
      std::string path = argv[1];
      if (path[path.size() - 1] != '/')
        path += '/';
      scores_file = path + scores_file;
      monthly_leaderboard_levels_file = path + monthly_leaderboard_levels_file;
      monthly_leaderboard_output_file = path + monthly_leaderboard_output_file;
      era2_leaderboard_output_file = path + era2_leaderboard_output_file;
      era2_leaderboard_output_file_1p = path + era2_leaderboard_output_file_1p;
      era2_leaderboard_output_file_2p = path + era2_leaderboard_output_file_2p;
    }
    else
      std::cout << "Path wasn't provided, using local directory as a root instead." << std::endl;

  if (load_scores())
    return 1;
  if (load_monthly_leaderboard_levels())
    return 1;
  process_scores();
  create_monthly_leaderboard();
  if (extract_monthly_leaderboard())
    return 1;
  //create_era2_leaderboards();
  //if (extract_era2_leaderboards())
  //  return 1;

  return 0;
}