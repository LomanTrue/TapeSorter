#include "FileTape.h"
#include "TapeConfig.h"
#include "TapeSorter.h"

#include <cstddef>
#include <cstdlib>
#include <exception>
#include <filesystem>
#include <iostream>
#include <memory>
#include <string>

namespace fs = std::filesystem;

struct Args {
  std::string input;
  std::string output;
  std::string configPath = "config/tape.conf";
  std::size_t memoryBytes = 4096;
};

bool parseArgs(int argc, char** argv, Args& args) {
  if (argc < 3) return false;
  args.input = argv[1];
  args.output = argv[2];
  for (int i = 3; i < argc; ++i) {
    const std::string flag = argv[i];
    if ((flag == "--config" || flag == "-c") && i + 1 < argc) {
      args.configPath = argv[++i];
    } else if ((flag == "--memory" || flag == "-m") && i + 1 < argc) {
      try {
          args.memoryBytes = std::stoull(argv[++i]);
      } catch (...) {
          return false;
      }
    } else {
      return false;
    }
  }
  return true;
}

int main(int argc, char** argv) {
  Args args;
  if (!parseArgs(argc, argv, args)) {
    return 1;
  }

  try {
    const TapeConfig config = TapeConfig::LoadFromFile(args.configPath);

    FileTape inputTape(args.input, config);
    const std::size_t length = inputTape.length();

    FileTape outputTape(args.output, length, config);

    std::size_t chunkSize = args.memoryBytes / sizeof(int32_t);
    if (chunkSize == 0) chunkSize = 1;

    const fs::path tmp_dir = "tmp";
    fs::create_directories(tmp_dir);

    TapeSorter::TempTapeFactory factory =
      [&tmp_dir, &config](std::size_t index, std::size_t len) -> std::unique_ptr<ITape> {
      const fs::path path = tmp_dir / ("run_" + std::to_string(index) + ".tape");
      return std::make_unique<FileTape>(path.string(), len, config);
    };

    TapeSorter sorter(chunkSize, factory);
    sorter.Sort(inputTape, outputTape);

    std::cout << "Sorted " << length << " elements from \"" << args.input
              << "\" to \"" << args.output << "\" (chunk = " << chunkSize
              << " elements).\n";
    return 0;
  } catch (const std::exception& e) {
    std::cerr << "Error: " << e.what() << "\n";
    return 1;
  }
}