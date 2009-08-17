#include "board-utils.h"

BoardUtils::BoardUtils(int w, int h, int num_classes)
    : w_(w), h_(h), num_classes_(num_classes) {
}

void BoardUtils::UsePartition(const std::vector<std::string>& letters) {
  // TODO(danvk): check that letters is actually a partition.
  classes_ = letters;
  num_classes_ = classes_.size();
}

// Not clear what this means in the context of board classes...
uint64_t BoardUtils::BoardId(const std::string& board) {
  if (board.size() != w_ * h_) return (uint64_t)-1;
  uint64_t id = 0;
  for (int i = 0; i < board.size(); i++) {
    if (board[i] < 'a' || board[i] >= 'a' + num_classes_)
      return (uint64_t)-1;
    id *= num_classes_;
    id += (board[i] - 'a');
  }
  return id;
}

const std::string BoardUtils::BoardFromId(uint64_t id) {
  std::string out(w_ * h_, ' ');
  uint64_t left = id;
  for (int i = w_ * h_ - 1; i >= 0; i--) {
    int idx = left % num_classes_;
    left /= num_classes_;
    out[i] = 'a' + idx;
  }
  return out;
}

bool BoardUtils::GenerateAnalogues(const std::string& board,
                                   std::vector<std::string>* analogues) {
  if (board.size() != w_ * h_) return false;
  analogues->clear();
  std::string bd;
  bd = FlipLeftRight(board); if (board != bd) analogues->push_back(bd);
  bd = FlipTopBottom(bd);    if (board != bd) analogues->push_back(bd);
  bd = FlipLeftRight(bd);    if (board != bd) analogues->push_back(bd);

  if (w_ == h_) {
    bd = Rotate90CW(board);  if (board != bd) analogues->push_back(bd);
    bd = FlipLeftRight(bd);  if (board != bd) analogues->push_back(bd);
    bd = FlipTopBottom(bd);  if (board != bd) analogues->push_back(bd);
    bd = FlipLeftRight(bd);  if (board != bd) analogues->push_back(bd);
  }

  analogues->erase(std::unique(analogues->begin(), analogues->end()),
                   analogues->end());
  return true;
}

std::string BoardUtils::FlipTopBottom(const std::string& bd) {
  if (bd.size() != w_ * h_) return "";
  std::string out(w_ * h_, ' ');
  for (int y = 0; y < h_; y++) {
    for (int x = 0; x < w_; x++) {
      out[Id(x, y)] = bd[Id(x, h_ - 1 - y)];
    }
  }
  return out;
}

std::string BoardUtils::FlipLeftRight(const std::string& bd) {
  if (bd.size() != w_ * h_) return "";
  std::string out(w_ * h_, ' ');
  for (int x = 0; x < w_; x++) {
    if (w_ - 1 - x == x) continue;
    for (int y = 0; y < h_; y++) {
    out[Id(x, y)] = bd[Id(w_ - 1 - x, y)];
    }
  }
  return out;
}

std::string BoardUtils::Rotate90CW(const std::string& bd) {
  if (w_ != h_) return "";  // rotation only works for square boards!
  std::string out(w_ * h_, ' ');
  for (int x = 0; x < w_; x++) {
    for (int y = 0; y < h_; y++) {
      out[Id(w_ - 1 - y, x)] = bd[Id(x,y)];
    }
  }
  return out;
}

bool BoardUtils::IsCanonical(const std::string& board) {
  if (board.size() != w_ * h_) return false;
  std::string bd;
  bd = FlipLeftRight(board); if (bd < board) return false;
  bd = FlipTopBottom(bd);    if (bd < board) return false;
  bd = FlipLeftRight(bd);    if (bd < board) return false;

  if (w_ == h_) {
    bd = Rotate90CW(board);  if (bd < board) return false;
    bd = FlipLeftRight(bd);  if (bd < board) return false;
    bd = FlipTopBottom(bd);  if (bd < board) return false;
    bd = FlipLeftRight(bd);  if (bd < board) return false;
  }
  return true;
}

std::string BoardUtils::Canonicalize(const std::string& board) {
  std::vector<std::string> rots;
  if (!GenerateAnalogues(board, &rots)) return "";
  rots.push_back(board);
  sort(rots.begin(), rots.end());
  return rots[0];
}

int BoardUtils::Id(int x, int y) { return y * w_ + x; }
int BoardUtils::X(int id) { return id % w_; }
int BoardUtils::Y(int id) { return id / w_; }


std::string BoardUtils::ExpandPartitions(const std::string& board) {
  if (board.size() != w_ * h_) return "";
  std::string out;
  for (int i = 0; i < board.size(); i++) {
    if (i) out += " ";
    if (board[i] < 'a' || board[i] >= 'a' + num_classes_) return "";
    out += classes_[board[i] - 'a'];
  }
  return out;
}
