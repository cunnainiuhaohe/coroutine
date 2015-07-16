#include "test.h"

#define NR_COROUTINE 2
crt_ctx_t *ctx[NR_COROUTINE];
static unsigned long long seq = 0;
static int co1_run = 0;
static int co2_run = 0;

void func1(void *str) {
  int i;

  for (i = 0; i < 10; ++i) {
    ASSERT(i == seq / 2, "resume fail");
    ++seq, ++co1_run;
    crt_resume(ctx[1]);
  }

  return;
}

void func2(void *str) {
  int i;

  for (i = 0; i < 10; ++i) {
    ASSERT(i == seq / 2, "resume fail");
    ++seq, ++co2_run;
    crt_resume(ctx[0]);
  }

  return;
}


TEST_IMPL(crt_resume) {
  crt_init();

  ctx[0] = crt_create(NULL, func1, str);
  ctx[1] = crt_create(NULL, func2, str);

  crt_resume(ctx[0]);
  ASSERT(co1_run == 10, "crt1 NOT run enough");
  ASSERT(co2_run == 10, "crt2 NOT run enough");

  return 0;
}
