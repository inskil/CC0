#include "argparse.hpp"
#include "fmt/core.h"
#include "src/file.h"
#include "src/file.cpp"
#include "tokenizer/tokenizer.h"
#include "analyser/analyser.h"
#include "fmts.hpp"
#include "src/vm.h"
#include "src/vm.cpp"

#include <iostream>
#include <fstream>

std::vector<cc0::Token> _tokenize(std::istream &input) {
    cc0::Tokenizer tkz(input);
    auto p = tkz.AllTokens();
    if (p.second.has_value()) {
        fmt::print(stderr, "Tokenization error: {}\n", p.second.value());
        exit(2);
    }
    return p.first;
}

void Tokenize(std::istream &input, std::ostream &output) {
    auto v = _tokenize(input);
    for (auto &it : v)
        output << fmt::format("{}\n", it);
    return;
}

void Analyse(std::istream &input, std::ostream &output) {
    auto tks = _tokenize(input);
    cc0::Analyser analyser(tks);
    auto p = analyser.Analyse();
    if (p.second.has_value()) {
        fmt::print(stderr, "Syntactic analysis error: {}\n", p.second.value());
        exit(2);
    }
    auto c = p.first.first;
    output << ".constants:\n";
    for (auto &it : c)
        output << fmt::format("{}\n", it);
    int32_t index = 0;
    output << ".start:\n";
    for (auto &it : analyser.getStart())
        output << index++ << "    " << fmt::format("{}\n", it);

    output << ".functions:\n";
    auto f = p.first.second;
    for (auto &it : f)
        output << fmt::format("{}\n", it);

    for (auto &it : f) {
        index = 0;
        output << ".F" << it.getIndex() << ":\n";
        for (auto &ins : it.getInstructions())
            output << index++ << "    " << fmt::format("{}\n", ins);
    }

    return;
}

void assemble_text(std::ifstream *in, std::ofstream *out) {
    try {
        File f = File::parse_file_text(*in);
        // f.output_text(std::cout);
        f.output_binary(*out);
    }
    catch (const std::exception &e) {
        println(std::cerr, e.what());
    }
}

void execute(std::ifstream *in, std::ostream *out) {
    try {
        File f = File::parse_file_binary(*in);
        auto avm = std::move(vm::VM::make_vm(f));
        avm->start();
    }
    catch (const std::exception &e) {
        println(std::cerr, e.what());
    }
}


int main(int argc, char **argv) {
    argparse::ArgumentParser program("cc0");
    program.add_argument("input")
            .help("speicify the file to be compiled.");
    program.add_argument("-t")
            .default_value(false)
            .implicit_value(true)
            .help("perform tokenization for the input file to text file.");
    program.add_argument("-s")
            .default_value(false)
            .implicit_value(true)
            .help("perform syntactic analysis for the input file to text file.");
    program.add_argument("-c")
            .default_value(false)
            .implicit_value(true)
            .help("perform syntactic analysis for the input file to binary file.");
    program.add_argument("-o", "--output")
            .default_value(std::string("out"))
            .help("specify the output file.");
    program.add_argument("-r")
            .default_value(false)
            .implicit_value(true)
            .help("Run you code input file directly.");

    try {
        program.parse_args(argc, argv);
    }
    catch (const std::runtime_error &err) {
        fmt::print(stderr, "{}\n\n", err.what());
        program.print_help();
        exit(2);
    }

    auto input_file = program.get<std::string>("input");
    auto output_file = program.get<std::string>("--output");
    std::istream *input;
    std::ostream *output;
    std::ifstream *cache;
    std::ifstream inf;
    std::ifstream infcache;
    std::ofstream outcache;
    std::ofstream outf;
    if (input_file != "-") {
        inf.open(input_file, std::ios::in);
        if (!inf) {
            fmt::print(stderr, "Fail to open {} for reading.\n", input_file);
            exit(2);
        }
        input = &inf;
    } else
        input = &std::cin;
    if (output_file != "-" && program["-c"] == false && program["-r"] == false) {
        outf.open(output_file, std::ios::out | std::ios::trunc);
        if (!outf) {
            fmt::print(stderr, "Fail to open {} for writing.\n", output_file);
            exit(2);
        }
        output = &outf;
    } else {
        output = &std::cout;
    }
    if (program["-t"] == true && (program["-s"] == true || program["-c"] == true)) {
        fmt::print(stderr, "You can only perform tokenization or syntactic analysis at one time.");
        exit(2);
    }
    if (program["-s"] == true && program["-c"] == true) {
        fmt::print(stderr, "只能选择一种输出方式谢谢。");
        exit(2);
    }
    if (program["-t"] == true && (program["-s"] == true || program["-c"] == true)) {
        fmt::print(stderr, "You can only perform tokenization or syntactic analysis at one time.");
        exit(2);
    }
    if (program["-t"] == true) {
        Tokenize(*input, *output);
    } else if (program["-s"] == true) {
        Analyse(*input, *output);
    } else if (program["-c"] == true || program["-r"] == true) {
        outcache.open("cache", std::ios::out | std::ios::trunc);
        if (!outcache) {
            fmt::print(stderr, "Fail to open {} for writing.\n", output_file);
            exit(2);
        }
        output = &outcache;

        Analyse(*input, *output);
        outcache.close();

        infcache.open("cache", std::ios::in);
        cache = &infcache;
        outf.open(output_file, std::ios::binary | std::ios::out | std::ios::trunc);
        if (!outf) {
            inf.close();
            exit(2);
        }
        output = &outf;
        assemble_text(cache, dynamic_cast<std::ofstream *>(output));
    } else {
        inf.close();
        infcache.close();
        outcache.close();
        outf.close();
        fmt::print(stderr, "You must choose tokenization or syntactic analysis.");
        exit(2);
    }
    if (program["-r"] == true) {
        outf.close();
        infcache.close();
        infcache.open(output_file, std::ios::binary | std::ios::in);
        if (!infcache) {
            exit(2);
        }
        cache = &infcache;
        output = &std::cout;
        execute(cache, output);

    }
    inf.close();
    infcache.close();
    outcache.close();
    outf.close();
    return 0;
}