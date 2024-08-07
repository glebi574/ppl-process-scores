#include <iostream>
#include <algorithm>
#include <string>
#include <regex>
#include <cmath>
#include <vector>
#include <unordered_map>
#include <fstream>
#include <filesystem>

static std::unordered_map<std::string, int> level_uuid_mapping;
static std::unordered_map<std::string, int> level_name_mapping;
static std::unordered_map<std::string, int> level_raw_name_mapping;
static std::unordered_map<std::string, int> account_uuid_mapping;
static std::unordered_map<std::string, int> account_name_mapping;
static std::unordered_map<std::string, int> account_raw_name_mapping;
static std::vector<std::string> level_names;
static std::vector<std::string> account_names;
static std::vector<std::string> account_uuids;

static std::regex hex_color_pattern("#[0-9a-fA-F]{8}");
std::string get_raw_name(std::string& str) {
  std::string name = str;
  return std::regex_replace(name, hex_color_pattern, "");
}

struct BaseScore { // base score, used to temporarily store data
  int account1_id; // 1st player id
  int account2_id = -1; // 2nd player id
  int level_id; // local level id
  int level_version; // level version
  int value; // score value
  int type; // score type 0 - usual, 1 - speedrun
  uint64_t timestamp; // timestamp
  std::string country; // country 2 letters

  BaseScore(std::vector<std::string>& data) {
    size_t pos;
    if ((pos = data[0].find(',')) == std::string::npos)
      account1_id = account_uuid_mapping[data[0]];
    else {
      account1_id = account_uuid_mapping[data[0].substr(0, pos)];
      account2_id = account_uuid_mapping[data[0].substr(pos + 1, std::string::npos)];
    }
    level_id = level_uuid_mapping[data[1]];
    level_version = std::stoi(data[2]);
    value = std::stoi(data[3]);
    type = std::stoi(data[4]);
    timestamp = std::stoul(data[5]);
    if (data.size() == 7)
      country = data[6];
  }
};

struct Score1p {
  int account_id; // player id
  int level_id; // local level id
  int value; // score value
  uint64_t timestamp; // timestamp
  std::string country; // country 2 letters

  Score1p(BaseScore& score) {
    account_id = score.account1_id;
    level_id = score.level_id;
    value = score.value;
    timestamp = score.timestamp;
    country = score.country;
  }

  friend std::ostream& operator<<(std::ostream&, const Score1p&);
};

std::ostream& operator<<(std::ostream& os, const Score1p& score) {
  os << get_raw_name(level_names[score.level_id]) << " " << get_raw_name(account_names[score.account_id]) << " " << score.value;
  return os;
}

struct Score2p {
  int account1_id; // 1st player id
  int account2_id; // 2nd player id
  int level_id; // local level id
  int value; // score value
  uint64_t timestamp; // timestamp

  Score2p(BaseScore& score) {
    account1_id = score.account1_id;
    account2_id = score.account2_id;
    level_id = score.level_id;
    value = score.value;
    timestamp = score.timestamp;
  }
};

struct CustomScore {
  int account_id; // account id
  double value; // score vlaue
  std::string country; // country 2 letters

  CustomScore() {}

  CustomScore(Score1p& score) {
    account_id = score.account_id;
    country = score.country;
  }
};

static std::string input_path, output_path;
static std::string level_data_path = "level_data.csv";
static std::string account_data_path = "account_data.csv";
static std::string score_data_path = "score_data.csv";
static std::string monthly_leaderboard_levels_path = "monthly_leaderboard_levels.txt";

static std::string monthly_leaderboard_path = "monthly_leaderboard.csv";

static std::vector<Score1p> scores_1p;
static std::vector<Score1p> speedrun_scores_1p;
static std::vector<Score2p> scores_2p;
static std::vector<Score2p> speedrun_scores_2p;

static std::vector<std::vector<Score1p>> level_leaderboards_1p;
static std::vector<std::vector<Score1p>> speedrun_level_leaderboards_1p;
static std::vector<std::vector<Score2p>> level_leaderboards_2p;
static std::vector<std::vector<Score2p>> speedrun_level_leaderboards_2p;

static std::vector<int> monthly_leaderboard_levels;

static std::vector<CustomScore> monthly_leaderboard;

static int level_counter = 0;
static int account_counter = 0;

template <typename T>
int get_index_of_score_by(std::vector<T>& leaderboard, int account_id) {
  for (int i = 0; i < leaderboard.size(); ++i)
    if (leaderboard[i].account_id == account_id)
      return i;
  return -1;
}

void fix_line(std::string& line) { // to fix line ending issues
  line.erase(std::remove(line.begin(), line.end(), '\r'), line.end());
}

std::vector<std::string> split_line(std::string& line) {
  fix_line(line);
  std::vector<std::string> strings;
  std::string str = "";
  bool in_quotes = false;
  for (char c : line)
    if (c == '\"')
      in_quotes = !in_quotes;
    else if (c == ',' && !in_quotes) {
      str.erase(std::remove(str.begin(), str.end(), '\"'), str.end());
      strings.push_back(str);
      str = "";
    }
    else
      str += c;
  if (line.back() != ',') { // issues in their database
    str.erase(std::remove(str.begin(), str.end(), '\"'), str.end());
    strings.push_back(str);
  }
  return strings;
}

int open_fstream(std::string& path, std::ifstream& __fstream) {
  if (!std::filesystem::exists(path)) {
    std::cout << "Can't find " << path << std::endl;
    return 1;
  }
  __fstream = std::ifstream(path);
  if (!__fstream.is_open()) {
    std::cout << "Failed to open " << path << std::endl;
    return 1;
  }
  std::cout << "Loading " << path << std::endl;
  return 0;
}

int open_fstream(std::string& path, std::ofstream& __fstream) {
  __fstream = std::ofstream(path);
  if (!__fstream.is_open()) {
    std::cout << "Failed to open " << path << std::endl;
    return 1;
  }
  std::cout << "Writing to " << path << std::endl;
  return 0;
}

void skip_line(std::ifstream& __fstream) { // their scv files start with data names
  std::string str;
  std::getline(__fstream, str);
}

int load_data() { // loads scores
  account_uuid_mapping["0"] = account_counter;
  account_name_mapping["?"] = account_counter++;
  account_uuids.push_back("0");
  account_names.push_back("?");

  std::string line;
  std::ifstream level_mapping_fstream;
  if (open_fstream(level_data_path, level_mapping_fstream))
    return 1;
  skip_line(level_mapping_fstream);
  while (std::getline(level_mapping_fstream, line)) {
    std::vector<std::string> level_info = split_line(line);
    level_uuid_mapping[level_info[0]] = level_counter;
    level_name_mapping[level_info[1]] = level_counter;
    level_raw_name_mapping[get_raw_name(level_info[1])] = level_counter++;
    level_names.push_back(level_info[1]);
  }
  level_mapping_fstream.close();

  std::ifstream account_mapping_fstream;
  if (open_fstream(account_data_path, account_mapping_fstream))
    return 1;
  skip_line(account_mapping_fstream);
  while (std::getline(account_mapping_fstream, line)) {
    std::vector<std::string> account_info = split_line(line);
    account_uuid_mapping[account_info[0]] = account_counter;
    account_name_mapping[account_info[1]] = account_counter;
    account_raw_name_mapping[get_raw_name(account_info[1])] = account_counter++;
    account_uuids.push_back(account_info[0]);
    account_names.push_back(account_info[1]);
  }
  account_mapping_fstream.close();

  std::ifstream monthly_leaderboard_levels_fstream;
  if (open_fstream(monthly_leaderboard_levels_path, monthly_leaderboard_levels_fstream))
    return 1;
  while (std::getline(monthly_leaderboard_levels_fstream, line)) {
    fix_line(line);
    if (level_raw_name_mapping.find(line) == level_raw_name_mapping.end()) {
      std::cout << "Level with specified name doesn't exist: " << line << std::endl;
      return 1;
    }
    else
      monthly_leaderboard_levels.push_back(level_raw_name_mapping[line]);
  }
  monthly_leaderboard_levels_fstream.close();

  std::vector<BaseScore> initial_scores; // initial data
  std::vector<int> level_versions;
  for (int i = 0; i < level_counter; ++i)
    level_versions.push_back(0);
  std::ifstream score_data_fstream;
  if (open_fstream(score_data_path, score_data_fstream))
    return 1;
  skip_line(score_data_fstream);
  while (std::getline(score_data_fstream, line)) {
    std::vector<std::string> data = split_line(line);
    BaseScore base_score(data);
    initial_scores.push_back(base_score);
    if (base_score.level_version > level_versions[base_score.level_id])
      level_versions[base_score.level_id] = base_score.level_version;
  }
  score_data_fstream.close();

  for (BaseScore& base_score : initial_scores) {
    if (base_score.level_version < level_versions[base_score.level_id])
      continue;
    if (base_score.account2_id == -1)
      if (base_score.type)
        speedrun_scores_1p.push_back(Score1p(base_score));
      else
        scores_1p.push_back(Score1p(base_score));
    else
      if (base_score.type)
        speedrun_scores_2p.push_back(Score2p(base_score));
      else
        scores_2p.push_back(Score2p(base_score));
  }

  return 0;
}

template <typename T>
bool compare_score(T& a, T& b) {
  return a.value > b.value;
}

void create_level_leaderboards() { // creates leaderboards for every level
  for (int i = 0; i < level_counter; ++i) {
    level_leaderboards_1p.push_back({});
    speedrun_level_leaderboards_1p.push_back({});
    level_leaderboards_2p.push_back({});
    speedrun_level_leaderboards_2p.push_back({});
  }

  for (Score1p& score : scores_1p)
    level_leaderboards_1p[score.level_id].push_back(score);
  for (Score1p& score : speedrun_scores_1p)
    speedrun_level_leaderboards_1p[score.level_id].push_back(score);
  for (Score2p& score : scores_2p)
    level_leaderboards_2p[score.level_id].push_back(score);
  for (Score2p& score : speedrun_scores_2p)
    speedrun_level_leaderboards_2p[score.level_id].push_back(score);

  for (int i = 0; i < level_counter; ++i) {
    std::sort(level_leaderboards_1p[i].begin(), level_leaderboards_1p[i].end(), compare_score<Score1p>);
    std::sort(speedrun_level_leaderboards_1p[i].begin(), speedrun_level_leaderboards_1p[i].end(), compare_score<Score1p>);
    std::sort(level_leaderboards_2p[i].begin(), level_leaderboards_2p[i].end(), compare_score<Score2p>);
    std::sort(speedrun_level_leaderboards_2p[i].begin(), speedrun_level_leaderboards_2p[i].end(), compare_score<Score2p>);
  }
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

void create_monthly_leaderboard() {
  std::unordered_map<int, CustomScore> tmp_monthly_leaderboard;
  for (int level_id : monthly_leaderboard_levels) {
    std::vector<Score1p>& level_leaderboard = level_leaderboards_1p[level_id];
    for (int i = 0; i < 25 && i < level_leaderboard.size(); ++i)
      if (tmp_monthly_leaderboard.find(level_leaderboard[i].account_id) == tmp_monthly_leaderboard.end()) {
        CustomScore score(level_leaderboard[i]);
        score.value = get_monthly_leaderboard_score(i);
        tmp_monthly_leaderboard[score.account_id] = score;
      }
      else
        tmp_monthly_leaderboard[level_leaderboard[i].account_id].value += get_monthly_leaderboard_score(i);
  }
  for (auto& pair : tmp_monthly_leaderboard)
    monthly_leaderboard.push_back(pair.second);
  std::sort(monthly_leaderboard.begin(), monthly_leaderboard.end(), compare_score<CustomScore>);
}

const double __era2_r = 100.f * sqrt(2.f);
double get_era2_leaderboard_score(int rank, int player_amount) {
  return __era2_r * pow(player_amount, 1.f / 6.f) / sqrt(rank);
}

int get_wr_amount(std::vector<std::vector<Score1p>>& level_leaderboards, std::vector<int>& level_ids, int account_id) {
  int wr_amount = 0;
  for (int level_id : level_ids)
    if (level_leaderboards[level_id][0].account_id == account_id)
      ++wr_amount;
  return wr_amount;
}

double get_average_place(std::vector<std::vector<Score1p>>& level_leaderboards, std::vector<int>& level_ids, int account_id) {
  double average_place = 0;
  int place_amount = 0;
  int place = 0;
  for (int level_id : level_ids) {
    if ((place = get_index_of_score_by(level_leaderboards[level_id], account_id)) != -1 && place < 25) {
      average_place += place;
      ++place_amount;
    }
    //std::cout << get_raw_name(level_names[level_id]) << " " << get_raw_name(account_names[account_id]) << ": " << place << std::endl;
  }
  return place_amount == 0 ? 0 : average_place / place_amount + 1;
}

void print_places_by(std::vector<std::vector<Score1p>>& level_leaderboards, std::vector<int>& level_ids, int account_id) {
  for (int level_id : level_ids)
    std::cout << get_raw_name(level_names[level_id]) << " " << get_raw_name(account_names[account_id]) << ": " << get_index_of_score_by(level_leaderboards[level_id], account_id) << std::endl;
}

void print_level_leaderboard(std::vector<Score1p>& level_leaderboard) {
  for (int i = 0; i < level_leaderboard.size(); ++i)
    std::cout << i + 1 << ". " << level_leaderboard[i] << std::endl;
}

void create_leaderboards() {
  create_level_leaderboards();
  create_monthly_leaderboard();
}

int extract_monthly_leaderboard() {
  std::ofstream monthly_leaderboard_fstream;
  if (open_fstream(monthly_leaderboard_path, monthly_leaderboard_fstream))
    return 1;
  for (CustomScore& score : monthly_leaderboard) {
    monthly_leaderboard_fstream << std::fixed << std::setprecision(0)
      << account_uuids[score.account_id] << ",\""
      << account_names[score.account_id] << "\","
      << score.country << ','
      << score.value << ','
      << get_wr_amount(level_leaderboards_1p, monthly_leaderboard_levels, score.account_id) << ','
      << std::setprecision(4) << get_average_place(level_leaderboards_1p, monthly_leaderboard_levels, score.account_id) << std::endl;
  }
  monthly_leaderboard_fstream.close();

  return 0;
}

int extraxt_leaderboards() {
  if (extract_monthly_leaderboard())
    return 1;

  return 0;
}

void modify_path(std::string& path, std::string& str) {
  str = path + str;
}

static std::string arg_input_str = "-input_path=";
static std::string arg_output_str = "-output_path=";

int main(int argc, char* argv[]) {
  for (int i = 1; i < argc; ++i) {
    std::string arg = argv[i];
    if (arg.find(arg_input_str) == 0) {
      input_path = arg.substr(arg_input_str.size());
      if (input_path[input_path.size() - 1] != '/')
        input_path += '/';
      modify_path(input_path, score_data_path);
      modify_path(input_path, account_data_path);
      modify_path(input_path, level_data_path);
      modify_path(input_path, monthly_leaderboard_levels_path);
    } else if (arg.find(arg_output_str) == 0) {
      output_path = arg.substr(arg_output_str.size());
      if (output_path[output_path.size() - 1] != '/')
        output_path += '/';
      modify_path(output_path, monthly_leaderboard_path);
    }
  }
  

  if (load_data())
    return 1;
  create_leaderboards();
  if (extraxt_leaderboards())
    return 1;

  //print_places_by(level_leaderboards_1p, monthly_leaderboard_levels, account_raw_name_mapping["JF"]);
  //for (int level_id : monthly_leaderboard_levels)
  //  print_level_leaderboard(level_leaderboards_1p[level_id]);

  //std::cin.get();
  return 0;
}