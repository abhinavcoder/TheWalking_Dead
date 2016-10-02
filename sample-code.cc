/**
 * CMA-ES, Covariance Matrix Adaptation Evolution Strategy
 * Copyright (c) 2014 Inria
 * Author: Emmanuel Benazera <emmanuel.benazera@lri.fr>
 *
 * This file is part of libcmaes.
 *
 * libcmaes is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * libcmaes is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with libcmaes.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "cmaes.h"
#include <iostream>
#include <chrono>
#include <thread>
#include "bits/stdc++.h"
using namespace libcmaes;
using namespace std::chrono;
using namespace std;
// const double sq5 = std::sqrt(5.0);
const auto d_wait_time = std::chrono::seconds(10);
const auto d_wait_time2 = std::chrono::seconds(1);
map<string, double> map_params;
vector<string> which_params;
FitFunc fsphere = [](const double *x, const int N)
{
    string s = "/home/kgpkubs/git_nish/kgpkubssim3d/paramfiles/defaultParamstraining.txt";
    std::stringstream ss;
    ss << std::this_thread::get_id();
    s += ss.str();
    auto map_copy = map_params;
    int j = 0;
    for(const auto& i: which_params) {
        map_copy[i] = x[j++];
    }
    // delete the previous file of this name.
    ofstream fout(s);
    if(!fout) {
        cerr << "Failed to create file\n\n\n";
        throw "Failed to create file\n\n\n";
        exit(-1);
    }
    {
        for (const auto& i: map_copy) {
            fout << i.first << '\t' << i.second << '\n';
        }
    }
    fout.close();
    cerr << "successfully created file " << s << '\n';
    // }
    cerr << "Running agent\n";
    system(("sh /home/kgpkubs/git_nish/kgpkubssim3d/optimization/run.sh " + s + ' ' + ss.str()).c_str());
    // const int chk = system((string("./sample_start-optimization.sh ")+ s).c_str());
    // assert(chk == 0);
    // std::this_thread::sleep_for(d_wait_time);
    ifstream fl("/home/kgpkubs/git_nish/kgpkubssim3d/"+ss.str());
    // double score;
    // while(!fl.eof()) {
    //   // std::this_thread::sleep_for(d_wait_time2);
    //   fl.close();
    //   fl.open(s);
    // }
    double ret;
    assert(!fl.eof());
    fl >> ret;
    fstream scores("last_scores", std::fstream::app);
    assert(ret >=0);
    if (ret < 0) {
        throw std::runtime_error("\n\n\n\n\n\n\n\n:O\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n");
    }
    scores << ret << '\n';
    cerr << "score = " << ret << '\n';
    if ( !flag) {
        ret *= 2;
    }
    scores.close();
    return -ret;
};

// map<string, string> namedParams;
void LoadParams(const string &inputsFile)
{
    auto& namedParams = map_params;
    istream *input;
    ifstream infile;
    istringstream inString;

    infile.open(inputsFile.c_str(), ifstream::in);

    if (!infile) {
        cerr << "Could not open parameter file " << inputsFile << endl;
        exit(1);
    }

    input = &(infile);

    string name;
    bool fBlockComment = false;
    while (!input->eof()) {
        // Skip comments and empty lines
        std::string str;
        std::getline(*input, str);
        if (str.length() >= 2 && str.substr(0, 2) == "/*") {
            fBlockComment = true;
        } else if (str == "*/") {
            fBlockComment = false;
        }
        if (fBlockComment || str == "" || str[0] == '#') {
            continue;
        }

        // otherwise parse strings
        stringstream s(str);
        std::string key;
        std::string value;
        std::getline(s, key, '\t'); // read thru tab
        std::getline(s, value); // read thru newline
        if (value.empty()) {
            continue;
        }
        namedParams[key] = stod(value);
    }

    infile.close();
}

std::vector<double> init() {
    LoadParams("/home/kgpkubs/git_nish/kgpkubssim3d/paramfiles/defaultParams.txt");
    //   which_params = {"utwalk_max_step_size_angle",
    //   "utwalk_max_step_size_x",
    //   "utwalk_max_step_size_y",
    //  "utwalk_shift_amount",
    //  "utwalk_walk_height",
    //  "utwalk_step_height",
    //  "utwalk_fraction_on_ground",
    //  "utwalk_fraction_in_air",
    //  "utwalk_phase_length",
    //  "utwalk_pid_step_size_x",
    //  "utwalk_pid_step_size_y",
    //  "utwalk_pid_step_size_rot",
    //  "utwalk_fwd_offset",
    //  "utwalk_fwd_offset_factor",
    //  "utwalk_max_normal_com_error",
    // "utwalk_max_acceptable_com_error"};
    which_params = {
        "utwalk_max_step_size_angle",
        "utwalk_max_step_size_x",
        "utwalk_max_step_size_y",
        "utwalk_shift_amount",
        "utwalk_walk_height",
        "utwalk_step_height",
        "utwalk_fraction_on_ground",
        "utwalk_fraction_in_air",
        "utwalk_phase_length",
        "utwalk_fwd_offset",
        "utwalk_fwd_offset_factor"
    };
    std::vector<double> dim;
    for (auto i : which_params) {
        dim.push_back(map_params[i]);
    }
    return dim;
}


int main(int argc, char *argv[])
{
    // Delete it before start of program
    system("rm last_scores");
    std::vector<double> x0 = init();
    int dim = x0.size();
    double sigma = 0.1;
    //int lambda = 100; // offsprings at each generation.

    double lbounds[x0.size()],ubounds[x0.size()]; // arrays for lower and upper parameter bounds, respectively
    for (int i=0;i<x0.size();i++)
    {
        lbounds[i] = 0.0;
        ubounds[i] = 300.0;
    }
    // ubounds[0] = 3.14;
    // ubounds[3] = 100.0;
    // ubounds[6] = ubounds[7] = 1.0;
    // ubounds[8] = 1.0;
    // ubounds[9] = ubounds[10] = ubounds[11] = 1.0;
    // ubounds[12] = 50;
    // ubounds[13] = 1.0;
    // ubounds[14] = ubounds[15] = 20.0;
    ubounds[0] = 3.14;
    ubounds[3] = 100.0;
    ubounds[6] = ubounds[7] = 1.0;
    ubounds[8] = 1.0;
    ubounds[9] = 50;
    ubounds[10] = 1.0;

    GenoPheno<pwqBoundStrategy> gp(lbounds,ubounds,dim); // genotype / phenotype transform associated to bounds.
    CMAParameters<GenoPheno<pwqBoundStrategy>> cmaparams(x0,sigma,-1,0,gp); // -1 for automatically decided lambda, 0 is for random seeding of the internal generator.

    // CMAParameters<> cmaparams(x0,sigma, -1, 0, gp);
    cmaparams.set_mt_feval(true); // activates the parallel evaluation
    cmaparams.set_algo(aCMAES);
    //cmaparams._algo = BIPOP_CMAES;
    CMASolutions cmasols = cmaes<>(fsphere,cmaparams);
    std::cout << "best solution: ";
    cmasols.print(std::cout,0,gp)<<"\n";
    std::cout << "optimization took " << cmasols.elapsed_time() / 1000.0 << " seconds\n";
    return cmasols.run_status();
}