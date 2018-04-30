/*
 input_setter.h

 Copyright (c) 2014, 2015, 2016 Terumasa Tadano

 This file is distributed under the terms of the MIT license.
 Please see the file 'LICENCE.txt' in the root directory 
 or http://opensource.org/licenses/mit-license.php for information.
*/

#pragma once

//#include "alm.h"
#include "alm.h"
#include <string>

namespace ALM_NS
{
    class InputSetter
    {
    public:
        InputSetter();
        ~InputSetter();

        void deallocator(ALM *alm);
        void set_general_vars(ALM *alm,
                              std::string prefix,
                              std::string mode,
                              std::string str_disp_basis,
                              std::string str_magmom,
                              int nat,
                              int nkd,
                              int nsym,
                              int is_printsymmetry,
                              const int is_periodic[3],
                              bool trim_dispsign_for_evenfunc,
                              bool lspin,
                              bool print_hessian,
                              int noncollinear,
                              int trevsym,
                              const std::string *kdname,
                              const double * const *magmom,
                              double tolerance,
                              double tolerance_constraint);
        void set_cell_parameter(ALM *alm,
                                double a,
                                const double lavec_tmp[3][3]);
        void set_interaction_vars(ALM *alm,
                                  int maxorder,
                                  const int *nbody_include);
        void set_cutoff_radii(ALM *alm,
                              int maxorder,
                              int nkd,
                              const double * const * const *rcs);
        void set_fitting_vars(ALM *alm,
                              int ndata,
                              int nstart,
                              int nend,
                              std::string dfile,
                              std::string ffile,
                              int constraint_flag,
                              std::string rotation_axis,
                              std::string fc2_file,
                              std::string fc3_file,
                              bool fix_harmonic,
                              bool fix_cubic,
                              int flag_sparse);
        void set_atomic_positions(ALM *alm,
                                  int nat,
                                  const int *kd,
                                  const double * const *xeq);
    };
}
