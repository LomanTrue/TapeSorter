#include <algorithm>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <memory>
#include <random>
#include <string>
#include <vector>

#include "MemoryTape.h"
#include "MiniTest.h"

#include "FileTape.h"
#include "TapeConfig.h"
#include "TapeSorter.h"

namespace fs = std::filesystem;

TEST(MemoryTape_read_write_basic) {
  MemoryTape t(3);
  ASSERT_TRUE(t.AtBegin());
  ASSERT_FALSE(t.AtEnd());

  t.Write(10);
  t.MoveForward();
  t.Write(20);
  t.MoveForward();
  t.Write(30);
  t.MoveForward();
  ASSERT_TRUE(t.AtEnd());

  t.Rewind();
  ASSERT_EQ(t.Read(), 10);
  t.MoveForward();
  ASSERT_EQ(t.Read(), 20);
  t.MoveForward();
  ASSERT_EQ(t.Read(), 30);
}

TEST(MemoryTape_move_backward) {
  MemoryTape t({1, 2, 3});
  t.MoveForward();
  t.MoveForward();
  ASSERT_EQ(t.Read(), 3);
  ASSERT_TRUE(t.MoveBackward());
  ASSERT_EQ(t.Read(), 2);
  ASSERT_TRUE(t.MoveBackward());
  ASSERT_TRUE(t.AtBegin());
  ASSERT_FALSE(t.MoveBackward());
}

TEST(TapeConfig_missing_file_returns_defaults) {
  const auto cfg = TapeConfig::LoadFromFile("does_not_exist_12345.conf");
  ASSERT_EQ(cfg.readDelay.count(), 0);
  ASSERT_EQ(cfg.writeDelay.count(), 0);
  ASSERT_EQ(cfg.moveDelay.count(), 0);
  ASSERT_EQ(cfg.rewindDelay.count(), 0);
}

TEST(TapeConfig_parses_key_value_pairs) {
  const std::string path = "tmp/test_config.conf";
  fs::create_directories("tmp");
  {
    std::ofstream f(path);
    f << "# comment line\n"
      << "read_delay_us = 10\n"
      << "write_delay_us=20\n"
      << "  move_delay_us  =  30  \n"
      << "rewind_delay_us=40\n"
      << "garbage line without equals\n"
      << "unknown_key=999\n";
  }
  const auto cfg = TapeConfig::LoadFromFile(path);
  ASSERT_EQ(cfg.readDelay.count(), 10);
  ASSERT_EQ(cfg.writeDelay.count(), 20);
  ASSERT_EQ(cfg.moveDelay.count(), 30);
  ASSERT_EQ(cfg.rewindDelay.count(), 40);
  fs::remove(path);
}

TEST(FileTape_create_write_read_roundtrip) {
  fs::create_directories("tmp");
  const std::string path = "tmp/test_filetape_rw.tape";
  fs::remove(path);
  TapeConfig cfg;

  {
    FileTape tape(path, 5, cfg);
    ASSERT_EQ(tape.length(), std::size_t{5});
    for (int i = 0; i < 5; ++i) {
      tape.Write((i + 1) * 100);
      tape.MoveForward();
    }
    ASSERT_TRUE(tape.AtEnd());
  }

  {
    FileTape tape(path, cfg);
    ASSERT_EQ(tape.length(), std::size_t{5});
    for (int i = 0; i < 5; ++i) {
      ASSERT_EQ(tape.Read(), (i + 1) * 100);
      tape.MoveForward();
    }
    ASSERT_TRUE(tape.AtEnd());
  }

  fs::remove(path);
}

TEST(FileTape_rewind_resets_position) {
  fs::create_directories("tmp");
  const std::string path = "tmp/test_filetape_rewind.tape";
  fs::remove(path);
  TapeConfig cfg;

  FileTape tape(path, 3, cfg);
  tape.Write(7);
  tape.MoveForward();
  tape.Write(8);
  tape.MoveForward();
  tape.Rewind();
  ASSERT_TRUE(tape.AtBegin());
  ASSERT_EQ(tape.Read(), 7);

  fs::remove(path);
}

TapeSorter::TempTapeFactory memoryFactory() {
  return [](std::size_t, std::size_t len) -> std::unique_ptr<ITape> {
    return std::make_unique<MemoryTape>(len);
  };
}

std::vector<int32_t> sortViaTapeSorter(std::vector<int32_t> input,
                                         std::size_t chunkSize) {
  MemoryTape in(input);
  MemoryTape out(input.size());
  TapeSorter sorter(chunkSize, memoryFactory());
  sorter.Sort(in, out);
  return out.data();
}


TEST(TapeSorter_empty_input) {
  auto result = sortViaTapeSorter({}, 4);
  ASSERT_TRUE(result.empty());
}

TEST(TapeSorter_single_element) {
  auto result = sortViaTapeSorter({42}, 4);
  ASSERT_EQ(result.size(), std::size_t{1});
  ASSERT_EQ(result[0], 42);
}

TEST(TapeSorter_already_sorted) {
  std::vector<int32_t> in = {1, 2, 3, 4, 5, 6, 7, 8};
  auto result = sortViaTapeSorter(in, 3);
  ASSERT_TRUE(result == in);
}

TEST(TapeSorter_reverse_sorted) {
  std::vector<int32_t> in = {9, 7, 5, 3, 1, -2, -4};
  auto result = sortViaTapeSorter(in, 2);
  auto expected = in;
  std::sort(expected.begin(), expected.end());
  ASSERT_TRUE(result == expected);
}

TEST(TapeSorter_with_duplicates) {
  std::vector<int32_t> in = {3, 1, 3, 2, 1, 2, 3, 1};
  auto result = sortViaTapeSorter(in, 3);
  auto expected = in;
  std::sort(expected.begin(), expected.end());
  ASSERT_TRUE(result == expected);
}

TEST(TapeSorter_chunk_size_1_degenerate) {
  std::vector<int32_t> in = {5, 1, 4, 2, 3};
  auto result = sortViaTapeSorter(in, 1);
  std::vector<int32_t> expected = {1, 2, 3, 4, 5};
  ASSERT_TRUE(result == expected);
}

TEST(TapeSorter_chunk_size_larger_than_input) {
  std::vector<int32_t> in = {3, 1, 4, 1, 5, 9, 2, 6};
  auto result = sortViaTapeSorter(in, 100);
  auto expected = in;
  std::sort(expected.begin(), expected.end());
  ASSERT_TRUE(result == expected);
}

TEST(TapeSorter_large_random) {
  constexpr std::size_t N = 10000;
  std::vector<int32_t> in(N);
  std::mt19937 rng(42);
  std::uniform_int_distribution<int32_t> dist(-1'000'000, 1'000'000);
  for (auto& x : in) x = dist(rng);

  auto result = sortViaTapeSorter(in, 137);
  auto expected = in;
  std::sort(expected.begin(), expected.end());
  ASSERT_TRUE(result == expected);
}

TEST(TapeSorter_end_to_end_with_FileTape) {
  fs::create_directories("tmp");
  const std::string inPath = "tmp/test_e2e_input.tape";
  const std::string outPath = "tmp/test_e2e_output.tape";
  fs::remove(inPath);
  fs::remove(outPath);

  std::vector<int32_t> in = {7, 2, 9, 1, 5, 3, 8, 6, 4, 0, -1, -2};
  TapeConfig cfg;

  {
    FileTape inputTape(inPath, in.size(), cfg);
    for (auto v : in) {
      inputTape.Write(v);
      inputTape.MoveForward();
    }
  }

  {
    FileTape inputTape(inPath, cfg);
    FileTape outputTape(outPath, in.size(), cfg);
    TapeSorter::TempTapeFactory factory =
      [&cfg](std::size_t i, std::size_t len) -> std::unique_ptr<ITape> {
      const std::string p = "tmp/test_e2e_run_" + std::to_string(i) + ".tape";
      return std::make_unique<FileTape>(p, len, cfg);
    };
    TapeSorter sorter(4, factory);
    sorter.Sort(inputTape, outputTape);
  }

  {
    FileTape outputTape(outPath, cfg);
    std::vector<int32_t> result;
    while (!outputTape.AtEnd()) {
      result.push_back(outputTape.Read());
      outputTape.MoveForward();
    }
    auto expected = in;
    std::sort(expected.begin(), expected.end());
    ASSERT_TRUE(result == expected);
  }

  fs::remove(inPath);
  fs::remove(outPath);
}

int main() {
  return RunAll();
}