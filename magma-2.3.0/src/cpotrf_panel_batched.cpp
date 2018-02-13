/*
    -- MAGMA (version 2.3.0) --
       Univ. of Tennessee, Knoxville
       Univ. of California, Berkeley
       Univ. of Colorado, Denver
       @date November 2017
       
       @author Azzam Haidar
       @author Tingxing Dong
       @author Ahmad Abdelfattah

       @generated from src/zpotrf_panel_batched.cpp, normal z -> c, Wed Nov 15 00:34:21 2017
*/
#include "magma_internal.h"
#include "batched_kernel_param.h"


/******************************************************************************/
extern "C" magma_int_t
magma_cpotrf_panel_batched(
    magma_uplo_t uplo, magma_int_t n, magma_int_t nb,     
    magmaFloatComplex** dA_array,    magma_int_t ldda,
    magmaFloatComplex** dX_array,    magma_int_t dX_length,
    magmaFloatComplex** dinvA_array, magma_int_t dinvA_length,
    magmaFloatComplex** dW0_displ, magmaFloatComplex** dW1_displ, 
    magmaFloatComplex** dW2_displ, magmaFloatComplex** dW3_displ,
    magmaFloatComplex** dW4_displ, 
    magma_int_t *info_array, magma_int_t gbstep,
    magma_int_t batchCount, magma_queue_t queue)
{
    magma_int_t arginfo = 0;
    //===============================================
    //  panel factorization
    //===============================================
    if (n < nb) {
        printf("magma_cpotrf_panel error n < nb %lld < %lld\n", (long long) n, (long long) nb );
        return -101;
    }

#if 0
    arginfo = magma_cpotf2_batched(
                       uplo, n, nb,
                       dA_array, ldda,
                       dW1_displ, dW2_displ,
                       dW3_displ, dW4_displ,
                       info_array, gbstep,
                       batchCount, queue);
#else
    arginfo = magma_cpotf2_batched(
                       uplo, nb, nb,
                       dA_array, ldda,
                       dW1_displ, dW2_displ,
                       dW3_displ, dW4_displ,
                       info_array, gbstep,
                       batchCount, queue);

    if ((n-nb) > 0) {
        magma_cdisplace_pointers(dW0_displ, dA_array, ldda, nb, 0, batchCount, queue);
        magmablas_ctrsm_work_batched( MagmaRight, MagmaLower, MagmaConjTrans, MagmaNonUnit,
                              1, n-nb, nb, 
                              MAGMA_C_ONE,
                              dA_array,    ldda, 
                              dW0_displ,   ldda, 
                              dX_array,    n-nb, 
                              dinvA_array, dinvA_length, 
                              dW1_displ,   dW2_displ, 
                              dW3_displ,   dW4_displ,
                              0, batchCount, queue );
    }
#endif
    return arginfo;
}


/******************************************************************************/
extern "C" magma_int_t
magma_cpotrf_recpanel_batched(
    magma_uplo_t uplo, magma_int_t m, magma_int_t n, 
    magma_int_t min_recpnb,    
    magmaFloatComplex** dA_array,    magma_int_t ldda,
    magmaFloatComplex** dX_array,    magma_int_t dX_length,
    magmaFloatComplex** dinvA_array, magma_int_t dinvA_length,
    magmaFloatComplex** dW0_displ, magmaFloatComplex** dW1_displ,  
    magmaFloatComplex** dW2_displ, magmaFloatComplex** dW3_displ,
    magmaFloatComplex** dW4_displ,
    magma_int_t *info_array, magma_int_t gbstep, 
    magma_int_t batchCount, magma_queue_t queue)
{
    magma_int_t arginfo = 0;
    // Quick return if possible
    if (m == 0 || n == 0) {
        return arginfo;
    }
    if (uplo == MagmaUpper) {
        printf("Upper side is unavailable\n");
        arginfo = -1;
        magma_xerbla( __func__, -(arginfo) );
        return arginfo;
    }
    if (m < n) {
        printf("error m < n %lld < %lld\n", (long long) m, (long long) n );
        arginfo = -101;
        magma_xerbla( __func__, -(arginfo) );
        return arginfo;
    }

    magmaFloatComplex **dA_displ  = NULL;
    magma_malloc((void**)&dA_displ,   batchCount * sizeof(*dA_displ));

    magmaFloatComplex alpha = MAGMA_C_NEG_ONE;
    magmaFloatComplex beta  = MAGMA_C_ONE;
    magma_int_t panel_nb = n;
    if (panel_nb <= min_recpnb) {
        //printf("calling bottom panel recursive with m=%d nb=%d\n",m,n);
        //  panel factorization
        magma_cdisplace_pointers(dA_displ, dA_array, ldda, 0, 0, batchCount, queue);
        //magma_cpotrf_rectile_batched(uplo, m, panel_nb, 16,
        arginfo = magma_cpotrf_panel_batched(uplo, m, panel_nb,
                           dA_displ, ldda,
                           dX_array, dX_length,
                           dinvA_array, dinvA_length,
                           dW0_displ, dW1_displ, dW2_displ,
                           dW3_displ, dW4_displ,
                           info_array, gbstep,
                           batchCount, queue);
    }
    else {
        // split A over two [A A2]
        // panel on A1, update on A2 then panel on A1    
        magma_int_t n1 = n/2;
        magma_int_t n2 = n-n1;
        magma_int_t m1 = m;
        magma_int_t m2 = m-n1;
        magma_int_t p1 = 0;
        magma_int_t p2 = n1;
        // panel on A1
        //printf("calling recursive panel on A1 with m=%d nb=%d min_recpnb %d\n",m1,n1,min_recpnb);
        magma_cdisplace_pointers(dA_displ, dA_array, ldda, p1, p1, batchCount, queue);        
        arginfo = magma_cpotrf_recpanel_batched(
                           uplo, m1, n1, min_recpnb,
                           dA_displ, ldda,
                           dX_array, dX_length,
                           dinvA_array, dinvA_length,
                           dW0_displ, dW1_displ, dW2_displ,
                           dW3_displ, dW4_displ, 
                           info_array, gbstep,
                           batchCount, queue);
        if (arginfo != 0) {
            magma_free(dA_displ);
            return arginfo;
        }

        // update A2
        //printf("calling update A2 with             m=%d n=%d k=%d\n",m2,n2,n1);
        magma_cdisplace_pointers(dA_displ,  dA_array, ldda, p1+n1, p1, batchCount, queue);        
        magma_cdisplace_pointers(dW0_displ, dA_array, ldda, p1+n1, p2, batchCount, queue);        
        magma_cgemm_batched( MagmaNoTrans, MagmaConjTrans, m2, n2, n1,
                             alpha, dA_displ, ldda, 
                             dA_displ, ldda, 
                             beta,  dW0_displ, ldda, 
                             batchCount, queue );
        // panel on A2
        //printf("calling recursive panel on A2 with m=%d nb=%d min_recpnb %d\n",m2,n2,min_recpnb);
        magma_cdisplace_pointers(dA_displ, dA_array, ldda, p2, p2, batchCount, queue);        
        arginfo = magma_cpotrf_recpanel_batched(
                                      uplo, m2, n2, min_recpnb,
                                      dA_displ, ldda,
                                      dX_array, dX_length,
                                      dinvA_array, dinvA_length,
                                      dW0_displ, dW1_displ, dW2_displ,
                                      dW3_displ, dW4_displ,
                                      info_array, gbstep,
                                      batchCount, queue);
    }

    magma_free(dA_displ);
    return arginfo;
}


/******************************************************************************/
extern "C" magma_int_t
magma_cpotrf_rectile_batched(
    magma_uplo_t uplo, magma_int_t m, magma_int_t n, 
    magma_int_t min_recpnb,    
    magmaFloatComplex** dA_array,    magma_int_t ldda,
    magmaFloatComplex** dX_array,    magma_int_t dX_length,
    magmaFloatComplex** dinvA_array, magma_int_t dinvA_length,
    magmaFloatComplex** dW0_displ, magmaFloatComplex** dW1_displ,  
    magmaFloatComplex** dW2_displ, magmaFloatComplex** dW3_displ,
    magmaFloatComplex** dW4_displ,
    magma_int_t *info_array, magma_int_t gbstep,
    magma_int_t batchCount, magma_queue_t queue)
{
    //magma_int_t DEBUG=0;

    // Quick return if possible
    if (m == 0 || n == 0) {
        return 1;
    }
    if (uplo == MagmaUpper) {
        printf("Upper side is unavailable\n");
        return -100;
    }
    if (m < n) {
        printf("error m < n %lld < %lld\n", (long long) m, (long long) n );
        return -101;
    }

    magmaFloatComplex **dA_displ  = NULL;
    magma_malloc((void**)&dA_displ,   batchCount * sizeof(*dA_displ));

    magmaFloatComplex alpha = MAGMA_C_NEG_ONE;
    magmaFloatComplex beta  = MAGMA_C_ONE;
    magma_int_t panel_nb = n;
    if (panel_nb <= min_recpnb) {
        // if (DEBUG == 1) printf("calling bottom panel recursive with n=%lld\n", (long long) panel_nb);
        //  panel factorization
        magma_cdisplace_pointers(dA_displ, dA_array, ldda, 0, 0, batchCount, queue);
        magma_cpotrf_panel_batched(
                           uplo, m, panel_nb,
                           dA_displ, ldda,
                           dX_array, dX_length,
                           dinvA_array, dinvA_length,
                           dW0_displ, dW1_displ, dW2_displ,
                           dW3_displ, dW4_displ,
                           info_array, gbstep,
                           batchCount, queue);
    }
    else {
        // split A over two [A11 A12;  A21 A22; A31 A32]
        // panel on tile A11, 
        // trsm on A21, using A11
        // update on A22 then panel on A22.  
        // finally a trsm on [A31 A32] using the whole [A11 A12; A21 A22]     
        magma_int_t n1 = n/2;
        magma_int_t n2 = n-n1;
        magma_int_t p1 = 0;
        magma_int_t p2 = n1;

        // panel on A11
        //if (DEBUG == 1) printf("calling recursive panel on A11=A(%d,%d) with n=%d min_recpnb %d\n", p1, p1, n1, min_recpnb);
        magma_cdisplace_pointers(dA_displ, dA_array, ldda, p1, p1, batchCount, queue);        
        magma_cpotrf_rectile_batched(
                           uplo, n1, n1, min_recpnb,
                           dA_displ, ldda,
                           dX_array, dX_length,
                           dinvA_array, dinvA_length,
                           dW0_displ, dW1_displ, dW2_displ,
                           dW3_displ, dW4_displ, 
                           info_array, gbstep,
                           batchCount, queue);

        // TRSM on A21
        //if (DEBUG == 1) printf("calling trsm on A21=A(%d,%d) using A11 == A(%d,%d) with m=%d k=%d\n",p2,p1,p1,p1,n2,n1);
        magma_cdisplace_pointers(dA_displ,  dA_array, ldda, p1, p1, batchCount, queue);        
        magma_cdisplace_pointers(dW0_displ, dA_array, ldda, p2, p1, batchCount, queue);
        magmablas_ctrsm_work_batched( MagmaRight, MagmaLower, MagmaConjTrans, MagmaNonUnit,
                              1, n2, n1, 
                              MAGMA_C_ONE,
                              dA_displ,    ldda, 
                              dW0_displ,   ldda, 
                              dX_array,    n2, 
                              dinvA_array, dinvA_length, 
                              dW1_displ,   dW2_displ, 
                              dW3_displ,   dW4_displ,
                              0, batchCount, queue );
        // update A22
        //if (DEBUG == 1) printf("calling update A22=A(%d,%d) using A21 == A(%d,%d) with m=%d n=%d k=%d\n",p2,p2,p2,p1,n2,n2,n1);
        magma_cdisplace_pointers(dA_displ,  dA_array, ldda, p2, p1, batchCount, queue);        
        magma_cdisplace_pointers(dW0_displ, dA_array, ldda, p2, p2, batchCount, queue);        // NEED TO BE REPLACED BY HERK
        magma_cgemm_batched( MagmaNoTrans, MagmaConjTrans, n2, n2, n1,
                             alpha, dA_displ, ldda, 
                             dA_displ, ldda, 
                             beta,  dW0_displ, ldda, 
                             batchCount, queue );

        // panel on A22
        //if (DEBUG == 1) printf("calling recursive panel on A22=A(%d,%d) with n=%d min_recpnb %d\n",p2,p2,n2,min_recpnb);
        magma_cdisplace_pointers(dA_displ, dA_array, ldda, p2, p2, batchCount, queue);        
        magma_cpotrf_rectile_batched(
                           uplo, n2, n2, min_recpnb,
                           dA_displ, ldda,
                           dX_array, dX_length,
                           dinvA_array, dinvA_length,
                           dW0_displ, dW1_displ, dW2_displ,
                           dW3_displ, dW4_displ, 
                           info_array, gbstep,
                           batchCount, queue);
    }

    if (m > n) {
        // TRSM on A3:
        //if (DEBUG == 1) printf("calling trsm AT THE END on A3=A(%d,%d): using A1222 == A(%d,%d) with m=%d k=%d\n",n,0,0,0,m-n,n);
        magma_cdisplace_pointers(dA_displ,  dA_array, ldda, 0, 0, batchCount, queue);        
        magma_cdisplace_pointers(dW0_displ, dA_array, ldda, n, 0, batchCount, queue);
        magmablas_ctrsm_work_batched( MagmaRight, MagmaLower, MagmaConjTrans, MagmaNonUnit,
                              1, m-n, n, 
                              MAGMA_C_ONE,
                              dA_displ,    ldda, 
                              dW0_displ,   ldda, 
                              dX_array,    m-n, 
                              dinvA_array, dinvA_length, 
                              dW1_displ,   dW2_displ, 
                              dW3_displ,   dW4_displ,
                              0, batchCount, queue );
    }

    magma_free(dA_displ);
    return 0;
}
