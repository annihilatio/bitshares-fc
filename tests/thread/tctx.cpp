
#include <stdio.h>

#include <boost/test/unit_test.hpp>

#include <boost/context/all.hpp>

namespace bc = boost::context;
namespace bcd = boost::context::detail;

BOOST_AUTO_TEST_SUITE(fc_context)

static uint64_t stack[1024*1024];

int r1, r2, r3;
static void run( bcd::transfer_t tfr )
{
    bcd::transfer_t tfr2;

    //printf("R1,fctx=%p,data=%p\n", tfr.fctx, tfr.data);

    int a[10];
    tfr2 = bcd::jump_fcontext( tfr.fctx, &r1);
    //printf("R2,fctx=%p,data=%p\n", tfr2.fctx, tfr2.data);

    int b[10];
    tfr2 = bcd::jump_fcontext( tfr2.fctx, &r2);
    //printf("R3,fctx=%p,data=%p\n", tfr2.fctx, tfr2.data);

    int c[10];
    tfr2 = bcd::jump_fcontext( tfr2.fctx, &r3);
    //printf("R4,fctx=%p,data=%p\n", tfr2.fctx, tfr2.data);
}

int d1, d2, d3;
BOOST_AUTO_TEST_CASE( use_boost_fcontext )
{
    //printf("P0\n");
    bcd::fcontext_t fctx = bcd::make_fcontext( stack + sizeof(stack)/sizeof(stack[0]), sizeof(stack), run );
    //printf("P1,fctx=%p\n", fctx);

    bcd::transfer_t tfr;
    tfr = bcd::jump_fcontext(     fctx, &d1);
    //printf("P2,fctx=%p,data=%p\n", tfr.fctx, tfr.data);

    int a[10];
    tfr = bcd::jump_fcontext(  tfr.fctx, &d2);
    //printf("P3,fctx=%p,data=%p\n", tfr.fctx, tfr.data);

    int b[10];
    tfr = bcd::jump_fcontext(  tfr.fctx, &d3);
    //printf("P4,fctx=%p,data=%p\n", tfr.fctx, tfr.data);

}

BOOST_AUTO_TEST_SUITE_END()
