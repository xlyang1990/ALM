#include <iostream>
#include <fstream>
#include <cmath>
#include <string>
#include <vector>
#include <set>
#include <boost/lexical_cast.hpp>
#include "fitting.h"
#include "files.h"
#include "error.h"
#include "memory.h"
#include "symmetry.h"
#include "system.h"
#include "fcs.h"
#include "interaction.h"
#include "timer.h"
#include "combination.h"
#include "constants.h"
#include <Eigen/Dense>

using namespace ALM_NS;

Fitting::Fitting(ALM *alm): Pointers(alm){}

Fitting::~Fitting() {
    memory->deallocate(params);
}

void Fitting::fitmain()
{
    files->ifs_disp_sym.open(files->file_disp_sym.c_str(), std::ios::in | std::ios::binary);
    if(!files->ifs_disp_sym) error->exit("fitmain", "cannot open file disp_sym");

    files->ifs_force_sym.open(files->file_force_sym.c_str(), std::ios::in | std::ios::binary);
    if(!files->ifs_force_sym) error->exit("fitmain", "cannot open file force_sym");


    int ntran = symmetry->ntran;
    int nat = system->nat;
    int natmin = symmetry->natmin;

    int i;
    int ndata = system->ndata;

    int N, M;
    int maxorder = interaction->maxorder;

    N = 0;
    for(i = 0; i < maxorder; ++i){
        N += fcs->ndup[i].size();
    }
    std::cout << "Total Number of Parameters : " << N << std::endl;
    M = 3 * natmin * ndata * ntran;

    memory->allocate(amat, M, N);
    memory->allocate(fsum, M);

    // calculate matrix elements for fitting
    calc_matrix_elements(M, N, nat, natmin, ntran, ndata, maxorder);
    timer->print_elapsed();

    // fitting with singular value decomposition or QR-Decomposition
    if(constraint == 0) {
        fit_without_constraints(M, N);
    } else {
        fit_with_constraints(M, N);
    }

    // copy force constants to public valiable "params"

    memory->allocate(params, N);

    for(i = 0; i < N; ++i)    params[i] = fsum[i];

    memory->deallocate(amat);
    memory->deallocate(fsum);

    timer->print_elapsed();
}

void Fitting::fit_without_constraints(int M, int N)
{
    int i, j, k;
    int nrhs = 1, nrank, INFO, LWORK;
    double *WORK, *S, *amat_mod;
    double rcond = -1.0;
    double f_square = 0.0;

    std::cout << "Entering Fitting Routine: SVD without constraints" << std::endl << std::endl;

    LWORK = 3 * std::min<int>(M, N) + std::max<int>(2*std::min<int>(M, N), std::max<int>(M, N));
    LWORK = 2 * LWORK;

    memory->allocate(WORK, LWORK);
    memory->allocate(S, N);

    // transpose matrix A
    memory->allocate(amat_mod, M * N);
    k = 0;
    for (j = 0; j < N; ++j){
        for(i = 0; i < M; ++i){
            amat_mod[k++] = amat[i][j];
        }
    }
    for (i = 0; i < M; ++i){
        f_square += std::pow(fsum[i], 2);
    }

    // fitting with singular value decomposition
    dgelss_(&M, &N, &nrhs, amat_mod, &M, fsum, &M, S, &rcond, &nrank, WORK, &LWORK, &INFO);

    std::cout << "Finished !" << std::endl << std::endl;

    std::cout << "RANK of the MATRIX: " << nrank << std::endl;
    if(nrank < N) error->warn("fit_without_constraints", 
        "Matrix is rank-deficient. Force constants could not be determined uniquely :(");

    if(nrank == N) {
        double f_residual = 0.0;
        for (i = N; i < M; ++i){
            f_residual += std::pow(fsum[i], 2);
        }
        std::cout << std::endl << "Residual sum of squares for the solution: " << sqrt(f_residual) << std::endl;
        std::cout << "Fitting Error (%) : "<< sqrt(f_residual/f_square) * 100.0 << std::endl;
    }

    memory->deallocate(amat_mod);
    memory->deallocate(WORK);
    memory->deallocate(S);
}

void Fitting::fit_with_constraints(int M, int N)
{
    int i, j, k;
    int nrank;
    int P, order, Pmax;
    double *mat_tmp;

    int maxorder = interaction->maxorder;

    double f_square, f_residual;

    std::cout << "Entering Fitting Routine: QRD with constraints" << std::endl << std::endl;
    memory->allocate(const_translation, maxorder);

    // generate constraints for translational invariance

    translational_invariance();

    // construct constraint matrix

    Pmax = 0;
    for (order = 0; order < maxorder; ++order){
        Pmax += const_translation[order].size();
    }

    if(constraint == 2){
        Pmax -= const_translation[0].size();
        Pmax += fcs->ndup[0].size();

    }
    memory->allocate(const_mat, Pmax, N);
    memory->allocate(const_rhs, Pmax);

    calc_constraint_matrix(N, P);

    std::cout << "Total number of constraints: " << P << std::endl;

    //for(i = 0; i < P; ++i){
    //    for(j = 0; j < N; ++j){
    //    std::cout << std::setw(4) << const_mat[i][j];
    //    }
    //    std::cout << std::setw(10) << const_rhs[i] << std::endl;
    //}

    memory->allocate(mat_tmp, (M + P) * N);
    k = 0;

    for(j = 0; j < N; ++j){
        for(i = 0; i < M; ++i){
            mat_tmp[k++] = amat[i][j];
        }
    }

    for(j = 0; j < N; ++j){
        for(i = 0; i < P; ++i){
            mat_tmp[k++] = const_mat[i][j];
        }
    }

    nrank = rank((M+P), N, mat_tmp);
    memory->deallocate(mat_tmp);

    if(nrank != N){
        std::cout << std::endl;
        std::cout << "!!WARNING: rank ( (A) ) ! = N" << std::endl;
        std::cout << "                ( (B) )      " << std::endl;
        std::cout << "rank = " << nrank << " N = " << N << std::endl;
        std::cout << "This must be a problem when solving equality constrained LSE problem with DGGLSE." << std::endl;
        std::cout << "Please change the cutoff radius or {u,f} data." << std::endl << std::endl;
    }

    f_square = 0.0;
    for (i = 0; i < M; ++i){
        f_square += std::pow(fsum[i], 2);
    }

    std::cout << "QR-Decomposition Started ...";

    double *amat_mod, *cmat_mod;
    memory->allocate(amat_mod, M * N);
    memory->allocate(cmat_mod, P * N);

    // transpose matrix A and C
    k = 0;
    for(j = 0; j < N; ++j){
        for(i = 0; i < M; ++i){
            amat_mod[k++] = amat[i][j];
        }
    }
    k = 0;
    for (j = 0; j < N; ++j){
        for(i = 0; i < P; ++i){
            cmat_mod[k++] = const_mat[i][j];
        }
    }

    // Fitting

    int LWORK = P + std::min<int>(M, N) + 100 * std::max<int>(M, N);
    int INFO;
    double *WORK, *x;
    memory->allocate(WORK, LWORK);
    memory->allocate(x, N);

    dgglse_(&M, &N, &P, amat_mod, &M, cmat_mod, &P, fsum, const_rhs, x, WORK, &LWORK, &INFO);

    memory->deallocate(amat_mod);
    memory->deallocate(cmat_mod);
    memory->deallocate(WORK);


    std::cout << " finished. " << std::endl;

    f_residual = 0.0;
    for (i = N - P; i < M; ++i){
        f_residual += std::pow(fsum[i], 2);
    }
    std::cout << std::endl << "Residual sum of squares for the solution: " << sqrt(f_residual) << std::endl;
    std::cout << "Fitting Error (%) : "<< sqrt(f_residual/f_square) * 100.0 << std::endl;

    // copy fcs to fsum

    for(i = 0; i < N; ++i){
        fsum[i] = x[i];
    }

    memory->deallocate(x);
    memory->deallocate(const_mat);
    memory->deallocate(const_rhs);
    memory->deallocate(const_translation);
}

void Fitting::calc_constraint_matrix(const int N, int &P){

    int i, j;
    int maxorder = interaction->maxorder;
    int order;
    int icol, irow;

    int nrow, ncol;

    int *nrank, *nparam;

    double *arr_tmp;

    std::vector<Constraint> *const_vec;

    memory->allocate(nrank, maxorder);
    memory->allocate(nparam, maxorder);
    memory->allocate(const_vec, maxorder);

    std::cout << "Removing redundant constraints ...";

    using namespace Eigen;

    for(order = 0; order < maxorder; ++order){

        const_vec[order].clear();

        nrow = fcs->ndup[order].size();
        ncol = const_translation[order].size();

        nparam[order] = nrow;

        memory->allocate(arr_tmp, nrow);

        MatrixXd mat_tmp(nrow, ncol);
        icol = 0;

        for (std::set<Constraint>::iterator p = const_translation[order].begin(); p != const_translation[order].end(); ++p){
            Constraint const_now = *p;
            for (i = 0; i < nrow; ++i){
                mat_tmp(i, icol) = const_now.w_const[i];
            }
            ++icol;
        }

        FullPivLU<MatrixXd> lu_decomp(mat_tmp);
        nrank[order] = lu_decomp.rank();
        MatrixXd c_reduced = lu_decomp.image(mat_tmp);

        for(icol = 0; icol < nrank[order]; ++icol){
            for(irow = 0; irow < nrow; ++irow){
                arr_tmp[irow] = c_reduced(irow, icol);
            }

            const_vec[order].push_back(Constraint(nrow, arr_tmp));
        }
        memory->deallocate(arr_tmp);
    }

    std::cout << " done." << std::endl << std::endl;

    std::cout << "Rank of the constraint matrices for each order ..." << std::endl;
    P = 0;
    for (order = 0; order < maxorder; ++order){
        P += nrank[order];
        std::cout << std::setw(9) << interaction->str_order[order] << ": " << const_vec[order].size() << std::endl;
    }
    std::cout << std::endl;

    if(constraint == 2) {
        std::cout << "Harmonic Force Constants will be fixed to the values in the given reference file: " << fc2_file << std::endl;
        std::cout << "Constraint Matrix for Harmonic fcs will be updated." << std::endl << std::endl;
        P =  P - nrank[0] + nparam[0];
    }

    for(i = 0; i < P; ++i){
        for(j = 0; j < N; ++j){
            const_mat[i][j] = 0.0;
        }
        const_rhs[i] = 0.0;
    }

    int minorder= 0;

    irow = 0;
    icol = 0;

    if(constraint == 2){
        std::ifstream ifs_fc2;
        ifs_fc2.open(fc2_file.c_str(), std::ios::in);
        if(!ifs_fc2) error->exit("calc_constraint_matrix", "cannot open file fc2_file");
        
        bool is_found = false;

        int nparam_harmonic;
        std::string str_tmp;
        while(!ifs_fc2.eof())
        {
            std::getline(ifs_fc2, str_tmp);
            
            if(str_tmp == "##HARMONIC FORCE CONSTANTS")
            {
                ifs_fc2 >> nparam_harmonic;
                if(nparam_harmonic != nparam[0]) error->exit("calc_constraint_matrix", "Number of fc2 not the same");

                is_found = true;

                for (i = 0; i < nparam[0]; ++i){
                    const_mat[i][i] = 1.0;
                    ifs_fc2 >> const_rhs[i];
                }
                break;
            }
        }
        minorder = 1;
        irow += nparam[0];
        icol += nparam[0];
        ifs_fc2.close();
        if(!is_found) error->exit("calc_constraint_matrix", "HARMONIC FORCE CONSTANTS flag not found in the fc2_file");
    }

    for(order = minorder; order < maxorder; ++order){

        for(std::vector<Constraint>::iterator it = const_vec[order].begin(); it != const_vec[order].end(); ++it){
            Constraint const_now = *it;

            for(i = 0; i < nparam[order]; ++i){
                const_mat[irow][i + icol] = const_now.w_const[i];
            }
            ++irow;
        }
        icol += nparam[order];
    }

    memory->deallocate(nrank);
    memory->deallocate(const_vec);
}


void Fitting::calc_matrix_elements(const int M, const int N, const int nat, const int natmin,
    const int ntran, const int ndata, const int maxorder)
{
    int i, j, k, order;
    int itran, iat;
    double **u;
    double **f;
    int *ind;

    std::cout << "Calculation of Matrix Elements for Direct Fitting Started ..." << std::endl;

    memory->allocate(ind, maxorder + 1);

    for (i = 0; i < M; ++i){
        for (j = 0; j < N; ++j){
            amat[i][j] = 0.0;
        }
        fsum[i] = 0.0;
    }

    memory->allocate(u, ntran, 3 * nat);
    memory->allocate(f, ntran, 3 * nat);

    int im = 0;
    int idata = 0;
    double amat_tmp;

    int iparam = 0;

    for(int data = 0; data < ndata; ++data){
        for (itran = 0; itran < ntran; ++itran){

            // read displacement and force

            for(i = 0; i < nat; ++i){
                for(j = 0; j < 3; ++j){
                    files->ifs_disp_sym.read((char *) &u[itran][3 * i + j], sizeof(double));
                    files->ifs_force_sym.read((char *) &f[itran][3 * i + j], sizeof(double));
                }
            }

            // construct r.h.s. vector B

            for (i = 0; i < natmin; ++i){
                iat = symmetry->map_p2s[i][0];
                for (j = 0; j < 3; ++j){
                    fsum[im++] = f[itran][3 * iat + j];
                }
            }

            //construct l.h.s. matrix A

            iparam = 0;

            for(order = 0; order < maxorder; ++order){

                int mm = 0;

                for(std::vector<int>::iterator iter = fcs->ndup[order].begin(); iter != fcs->ndup[order].end(); ++iter){
                    for (i = 0; i < *iter; ++i){
                        ind[0] = fcs->fc_set[order][mm].elems[0];
                        k = idata + inprim_index(fcs->fc_set[order][mm].elems[0]);
                        amat_tmp = 1.0;
                        for(j = 1; j < order + 2; ++j){
                            ind[j] = fcs->fc_set[order][mm].elems[j];
                            amat_tmp *= u[itran][fcs->fc_set[order][mm].elems[j]];
                        }
                        //	std::cout << "k = " << k << " iparam = " << iparam << std::endl;
                        amat[k][iparam] -= gamma(order + 2, ind) * fcs->fc_set[order][mm].coef * amat_tmp;
                        ++mm;
                    }
                    ++iparam;
                }
            }

            idata += 3 * natmin;
        }
    }
    memory->deallocate(ind);
    memory->deallocate(u);
    memory->deallocate(f);

    std::cout << " Finished !" << std::endl;
}

void Fitting::translational_invariance()
{
    // create constraint matrix arising from translational invariance.
    // might be a little tricky. (hard to follow)

    int i, j;
    int iat, jat, icrd, jcrd;
    int order;
    int maxorder = interaction->maxorder;

    int *ind;
    int *intarr, *intarr_copy;
    int **xyzcomponent;

    int nxyz;
    int natmin = symmetry->natmin;
    int nat = system->nat;

    double *arr_constraint;

    std::vector<int> intlist;
    std::set<FcProperty> list_found;
    std::set<FcProperty>::iterator iter_found;

    std::cout << "Start generating constraints for translational invariance ..." << std::endl;

    //FcProperty list_tmp;

    memory->allocate(ind, maxorder + 1);

    // const_translation = new std::set<Constraint> [maxorder];

    for (order = 0; order < maxorder; ++order){

        std::cout << std::setw(8) << interaction->str_order[order] << " ...";

        const_translation[order].clear();
        int nparams = fcs->ndup[order].size();

        if(nparams == 0) {
            std::cout << " skipped."<< std::endl;
            continue;
        }

        // make interaction list

        list_found.clear();
        for(std::vector<FcProperty>::iterator p = fcs->fc_set[order].begin(); p != fcs->fc_set[order].end(); ++p){
            FcProperty list_tmp = *p; //using copy constructor
            for (i = 0; i < order + 2; ++i){
                ind[i] = list_tmp.elems[i];
            }
            if(list_found.find(FcProperty(order + 2, list_tmp.coef, ind, list_tmp.mother)) != list_found.end()) {
                error->exit("translational invariance", "Duplicate interaction list found");
            }
            list_found.insert(FcProperty(order + 2, list_tmp.coef, ind, list_tmp.mother));
        }

        // generate xyz component for each order

        nxyz = static_cast<int>(pow(static_cast<long double>(3), order + 1));
        memory->allocate(xyzcomponent, nxyz, order + 1);
        fcs->get_xyzcomponent(order + 1, xyzcomponent);


        memory->allocate(arr_constraint, nparams);
        memory->allocate(intarr, order + 2);
        memory->allocate(intarr_copy, order + 2);

        for(i = 0; i < natmin; ++i){

            iat = symmetry->map_p2s[i][0];

            // generate atom pairs for each order

            if(order == 0){
                for(icrd = 0; icrd < 3; ++icrd){

                    intarr[0] = 3 * iat + icrd;

                    for (jcrd = 0; jcrd < 3; ++jcrd){

                        // Reset the temporary array for next constraint
                        for (j = 0; j < nparams; ++j) arr_constraint[j] = 0.0;

                        for (jat = 0; jat < 3 * nat; jat += 3){
                            intarr[1] = jat + jcrd;

                            iter_found = list_found.find(FcProperty(order + 2, 1.0, intarr, 1));

                            // corresponding fcs found.
                            if(iter_found != list_found.end()){
                                FcProperty arrtmp = *iter_found;
                                arr_constraint[arrtmp.mother] += arrtmp.coef;
                            }
                        }
                        if(!is_allzero(nparams,arr_constraint)){
                            // add to constraint list
                            const_translation[order].insert(Constraint(nparams, arr_constraint));
                        }
                    }
                }
            } else {
                for(j = 0; j < interaction->ninter[i][order]; ++j){
                    intlist.push_back(interaction->intpairs[i][order][j]);
                }
                std::sort(intlist.begin(), intlist.end());

                CombinationWithRepetition<int> g(intlist.begin(), intlist.end(), order);
                do {
                    std::vector<int> data = g.now();

                    intarr[0] = iat;
                    intarr[1] = data[0];
                    for (unsigned int isize = 1; isize < data.size(); ++isize){
                        intarr[isize + 1] = data[isize];
                    }

                    if(!interaction->is_incutoff(order + 1, intarr)) continue;

                    for(int ixyz = 0; ixyz < nxyz; ++ixyz){
                        for(int jcrd = 0; jcrd < 3; ++jcrd){

                            // Reset the temporary array for next constraint
                            for (j = 0; j < nparams; ++j) arr_constraint[j] = 0.0;

                            for(int jat = 0; jat < 3 * nat; jat += 3){
                                intarr[order + 1] = jat / 3;

                                if(!interaction->is_incutoff(order + 2, intarr)) continue;

                                for(j = 0; j < order + 1; ++j)  intarr_copy[j] = 3 * intarr[j] + xyzcomponent[ixyz][j];
                                intarr_copy[order + 1] = jat + jcrd;

                                fcs->sort_tail(order + 2, intarr_copy);

                                iter_found = list_found.find(FcProperty(order + 2, 1.0, intarr_copy, 1));
                                if(iter_found != list_found.end()){
                                    FcProperty arrtmp = *iter_found;
                                    arr_constraint[arrtmp.mother] += arrtmp.coef;                                
                                } 

                            }
                            if(!is_allzero(nparams,arr_constraint)){
                                const_translation[order].insert(Constraint(nparams, arr_constraint));
                            }
                        }
                    }

                } while(g.next());
                intlist.clear();
            }
        }

        memory->deallocate(xyzcomponent);
        memory->deallocate(arr_constraint);
        memory->deallocate(intarr);
        memory->deallocate(intarr_copy);

        std::cout << " done." << std::endl;
    }
    memory->deallocate(ind);

    std::cout << "Finished !" << std::endl << std::endl;
    for(order = 0;  order < maxorder; ++order){
        std::cout << "Number of Constraints for" << std::setw(9) << interaction->str_order[order] << " : " << const_translation[order].size() << std::endl;
    }
    std::cout << std::endl;
}

bool Fitting::is_allzero(const int n, const double *arr){

    for(int i = 0; i < n; ++i){
        if(std::abs(arr[i]) > eps10) {
            return false;
        }
    }
    return true;
}

int Fitting::inprim_index(const int n)
{
    int in;
    int atmn = n / 3;
    int crdn = n % 3;

    for (int i = 0; i < symmetry->natmin; ++i){
        if(symmetry->map_p2s[i][0] == atmn){
            in = 3 * i + crdn;
            break;
        }
    }
    return in;
}

double Fitting::gamma(const int n, const int *arr)
{
    int *arr_tmp, *nsame;
    int i;
    int ind_front, nsame_to_front;

    memory->allocate(arr_tmp, n);
    memory->allocate(nsame, n);

    for (i = 0; i < n; ++i) {
        arr_tmp[i] = arr[i];
        nsame[i] = 0;
    }

    ind_front = arr[0];
    nsame_to_front = 1;

    interaction->insort(n, arr_tmp);

    int nuniq = 1;
    int iuniq = 0;

    nsame[0] = 1;

    for(i = 1; i < n; ++i){
        if(arr_tmp[i] == arr_tmp[i-1]) {
            ++nsame[iuniq];
        } else {
            ++nsame[++iuniq];
            ++nuniq;
        }

        if(arr[i] == ind_front) ++nsame_to_front;
    }

    int denom = 1;

    for(i = 0; i < nuniq; ++i){
        denom *= factorial(nsame[i]);
    }

    memory->deallocate(arr_tmp);
    memory->deallocate(nsame);

    return static_cast<double>(nsame_to_front) / static_cast<double>(denom);
}

int Fitting::factorial(const int n)
{
    if(n == 1 || n == 0){
        return 1;
    }else{
        return n * factorial(n - 1);
    }
}

//int Fitting::rank(const int m, const int n, double **mat)
//{
//  using namespace Eigen;
// 
//    MatrixXd mat_tmp(m, n);
//
//    int i, j;
//
//    for(i = 0; i < m; ++i){
//        for(j = 0; j < n; ++j){
//            mat_tmp(i,j) = mat[i][j];
//        }
//    }
//    ColPivHouseholderQR<MatrixXd> qr(mat_tmp);
//    return qr.rank();
//}


/* unsuccessful
int Fitting::getRankEigen(int m, int p, int n)
{
using namespace Eigen;
MatrixXd mat(m + p, n);


int i, j;
int k = 0;
for(i = 0; i < m; ++i){
for(j = 0; j < n; ++j){
mat(k, j) = amat[i][j];
}
++k;
}
std::cout <<"OK";

for(i = 0; i < p; ++i){
for(j = 0; j < n; ++j){
mat(k, j) = const_mat[i][j];
}
++k;
}
std::cout <<"OK";
ColPivHouseholderQR<MatrixXd> qr(mat);
return qr.rank();
}
*/

//int Fitting::rank(int m, int n, double *mat)
//{
//    int LWORK = 10 * n;
//    int INFO;
//    double *WORK, *TAU;
//    int lda = m;
//    
//    int nmin = std::min<int>(m, n);
//
//    memory->allocate(WORK, LWORK);
//    memory->allocate(TAU, n);
//
//    dgeqrf_(&m, &n, mat, TAU, WORK, &LWORK, &INFO);
//
//
//}

int Fitting::rank(int m, int n, double *mat)
{
    int i;

    int LWORK = 10 * m;
    int INFO;
    int *IWORK;
    int ldu = 1, ldvt = 1;
    double *s, *WORK;
    double u[1], vt[1];

    int nmin = std::min<int>(m, n);

    memory->allocate(IWORK, 8 * nmin);
    memory->allocate(WORK, LWORK);
    memory->allocate(s, nmin);

    char mode[]  = "N";

    dgesdd_(mode, &m, &n, mat, &m, s, u, &ldu, vt, &ldvt, WORK, &LWORK, IWORK, &INFO); 

    int rank = 0;
    for(i = 0; i < nmin; ++i){
        if(s[i] > eps12) ++rank;
    }

    memory->deallocate(IWORK);
    memory->deallocate(s);

    return rank;
}
