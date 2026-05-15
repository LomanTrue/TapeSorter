# Tape Sort

Тестовое задание: внешняя сортировка целочисленных данных, хранящихся
на эмулируемой "ленте" (последовательное устройство хранения).

## Сборка

```
cmake -B build -S .
cmake --build build
```

Получаются два исполняемых файла:
- `build/tape_sort` — основное приложение
- `build/tape_sort_tests` — юнит-тесты

## Запуск

```
./build/tape_sort <input_file> <output_file> [--config <path>] [--memory <bytes>]
```

Пример:

```
./build/tape_sort data/input.bin data/output.bin --config config/tape.conf --memory 4096
```

По умолчанию `--config = config/tape.conf`, `--memory = 4096` байт.

Файлы — бинарные, по 4 байта на элемент (`int32_t`, little-endian
на x86/x64). Длина выходной ленты задаётся равной длине входной.

## Запуск тестов

```
./build/tape_sort_tests
# или через ctest:
cd build && ctest --output-on-failure
```
