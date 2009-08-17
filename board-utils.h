// Tools for generating boards and canonicalization.

#ifndef BOARD_UTILS
#define BOARD_UTILS

#include <string>
#include <vector>
#include <sys/types.h>
#include <stdint.h>

class BoardUtils {
 public:
  BoardUtils(int w, int h, int num_classes = 26);

  // Convert boards to/from a numerical representation.
  // 26^16 > 2^64, so these methods are only useful with partitions.
  uint64_t BoardId(const std::string& board);
  const std::string BoardFromId(uint64_t id);

  // Returns true if the board is in the "canonoical" rotation/reflection.
  bool IsCanonical(const std::string& board);

  // Returns the canonical version of the board (possibly the input board).
  std::string Canonicalize(const std::string& board);

  // Generates all boards in the same symmetry class.
  bool GenerateAnalogues(const std::string& board,
                         std::vector<std::string>* analogues);

  // Use a letter partition instead of all 26 letters.
  void UsePartition(const std::vector<std::string>& letters);

  // Usual representation is 'a' for class 1, 'b' for class 2, etc.
  // This expands a class like abc to the full "aeiou bcdefg zyxwv" rep.
  std::string ExpandPartitions(const std::string& board);

  // Basic symmetries applied to board strings
  std::string FlipTopBottom(const std::string& bd);
  std::string FlipLeftRight(const std::string& bd);
  std::string Rotate90CW(const std::string& bd);

 private:
  int Id(int x, int y);
  int X(int id);
  int Y(int id);

  int w_;
  int h_;
  int num_classes_;
  std::vector<std::string> classes_;
};

#endif
