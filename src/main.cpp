#include <chrono>
#include <fstream>
#include <iostream>
#include <filesystem>
#include <string>
#include <clocale>
#include <iterator>

#include <yapl/values/ArrayValue.hpp>
#include <yapl/values/FunctionValue.hpp>
#include <yapl/values/TypeObjectValue.hpp>

#include "yapl/ByteCodeVM.hpp"
#include "yapl/values/UndefinedValue.hpp"
#include "yapl/values/IntegerValue.hpp"
#include "yapl/values/FloatValue.hpp"
#include "yapl/values/BooleanValue.hpp"
#include "yapl/values/CodeObjectValue.hpp"
#include "yapl/Hasher.hpp"
#include "yapl/Module.hpp"
#include "yapl/Resolver.hpp"
#include "yapl/exceptions/DiagnosticsError.hpp"
#include "yapl/values/BuiltinFunctionValue.hpp"
#include "yapl/values/DictValue.hpp"
#include "yapl/values/ModuleValue.hpp"

using namespace yapl;

void initialize()
{
  init_builtin_function_tp();
  init_int_tp();
  init_float_type();
  init_bool_tp();
  init_str_tp();
  init_tp_tp();
  init_dict_tp();
  init_function_tp();
  init_size_iterator_type();
  init_undefined_tp();
  init_code_object_type_object();
  init_array_tp();
  init_module_type_object();
}

int run_diagnostics_mode(const std::filesystem::path &full_path)
{
  const auto filename = full_path.filename();

  try
  {
    auto module = Module{ full_path, filename.stem().string() };
    const auto parsed_unit = module.Resolve();
    const auto json = parsed_unit->SerializeJSON();

    std::cout << json << '\n';
    return 0;
  }
  catch (const std::exception &error)
  {
    std::cerr << error.what() << '\n';
    return 1;
  }
}

int run_diagnostics_stdin_mode(const std::filesystem::path &full_path)
{
  const auto filename = full_path.filename();

  try
  {
    std::string text(
      (std::istreambuf_iterator<char>(std::cin)),
      std::istreambuf_iterator<char>()
    );

    auto module = Module{ full_path, filename.stem().string(), text };
    const auto parsed_unit = module.Resolve();
    const auto json = parsed_unit->SerializeJSON();

    std::cout << json << '\n';
    return 0;
  }
  catch (const std::exception &error)
  {
    std::cerr << error.what() << '\n';
    return 1;
  }
}

int run_vm_mode(const std::filesystem::path &full_path)
{

  try
  {
    const auto path = full_path.parent_path();
    const auto filename = full_path.filename();

    auto module = Module{ path, filename.stem().string() };

    if (module.IsCacheValid())
    {
      module.DeserializeCache();
    }
    else
    {
      module.Tokenize();
      module.Parse();
      const auto result = module.Resolve();
      ThrowIfHasDiagnosticErrors(
        filename.string(),
        result->diagnostics,
        module.GetLines()
      );

      module.Generate();
      module.SerializeCache();

      ByteCodeVM VM;
      module.Run(VM, VM.modules["__builtins__"]);
      return 0;
    }
  }
  catch (const std::exception &error)
  {
    std::cerr << error.what() << std::endl;
    return 1;
  }
}

int main(int argc, char **argv)
{
  std::setlocale(LC_ALL, "ru_RU.utf-8");

  if (argc < 2)
  {
    std::cerr << "Usage:\n";
    std::cerr << "  yapl <file>\n";
    std::cerr << "  yapl --diagnostics <file>\n";
    std::cerr << "  yapl --diagnostics --stdin <file>\n";
    return 1;
  }

  initialize();

  const std::string first_argument = argv[1];

  if (first_argument == "--diagnostics")
  {
    if (argc == 3)
    {
      const auto full_path = std::filesystem::path(argv[2]);
      return run_diagnostics_mode(full_path);
    }

    if (argc == 4 && std::string(argv[2]) == "--stdin")
    {
      const auto full_path = std::filesystem::path(argv[3]);
      return run_diagnostics_stdin_mode(full_path);
    }

    std::cerr << "Usage:\n";
    std::cerr << "  yapl --diagnostics <file>\n";
    std::cerr << "  yapl --diagnostics --stdin <file>\n";
    return 1;
  }

  const auto full_path = std::filesystem::path(argv[1]);
  return run_vm_mode(full_path);
}
