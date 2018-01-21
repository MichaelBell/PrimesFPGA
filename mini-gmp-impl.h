typedef uint64_t mp_limb_t;
typedef int32_t mp_size_t;
typedef uint32_t mp_bitcnt_t;

typedef mp_limb_t *mp_ptr;
typedef const mp_limb_t *mp_srcptr;

typedef struct
{
  int _mp_alloc;		/* Number of *limbs* allocated and pointed
				   to by the _mp_d field.  */
  int _mp_size;			/* abs(_mp_size) is the number of limbs the
				   last field points to.  If _mp_size is
				   negative this is a negative number.  */
  mp_limb_t *_mp_d;		/* Pointer to the limbs.  */
} __mpz_struct;

struct gmp_div_inverse
{
	/* Normalization shift count. */
	unsigned shift;
	/* Normalized divisor (d0 unused for mpn_div_qr_1) */
	mp_limb_t d1, d0;
	/* Inverse, for 2/1 or 3/2. */
	mp_limb_t di;
};

typedef __mpz_struct mpz_t[1];

typedef __mpz_struct *mpz_ptr;
typedef const __mpz_struct *mpz_srcptr;

/* Macros */
#define GMP_LIMB_BITS (sizeof(mp_limb_t) * CHAR_BIT)

#define GMP_LIMB_MAX (~ (mp_limb_t) 0)
#define GMP_LIMB_HIGHBIT ((mp_limb_t) 1 << (GMP_LIMB_BITS - 1))

#define GMP_HLIMB_BIT ((mp_limb_t) 1 << (GMP_LIMB_BITS / 2))
#define GMP_LLIMB_MASK (GMP_HLIMB_BIT - 1)

#define GMP_ULONG_BITS (sizeof(unsigned long) * CHAR_BIT)
#define GMP_ULONG_HIGHBIT ((unsigned long) 1 << (GMP_ULONG_BITS - 1))

#define GMP_ABS(x) ((x) >= 0 ? (x) : -(x))
#define GMP_NEG_CAST(T,x) (-((T)((x) + 1) - 1))

#define GMP_MIN(a, b) ((a) < (b) ? (a) : (b))
#define GMP_MAX(a, b) ((a) > (b) ? (a) : (b))

#define GMP_CMP(a,b) (((a) > (b)) - ((a) < (b)))

#define gmp_assert_nocarry(x) do { \
    mp_limb_t __cy = (x);	   \
    assert (__cy == 0);		   \
  } while (0)

#define gmp_clz(count, x) do {						\
    mp_limb_t __clz_x = (x);						\
    unsigned __clz_c;							\
    for (__clz_c = 0;							\
	 (__clz_x & ((mp_limb_t) 0xff << (GMP_LIMB_BITS - 8))) == 0;	\
	 __clz_c += 8)							\
      __clz_x <<= 8;							\
    for (; (__clz_x & GMP_LIMB_HIGHBIT) == 0; __clz_c++)		\
      __clz_x <<= 1;							\
    (count) = __clz_c;							\
  } while (0)

#define gmp_ctz(count, x) do {						\
    mp_limb_t __ctz_x = (x);						\
    unsigned __ctz_c = 0;						\
    gmp_clz (__ctz_c, __ctz_x & - __ctz_x);				\
    (count) = GMP_LIMB_BITS - 1 - __ctz_c;				\
  } while (0)

#define gmp_add_ssaaaa(sh, sl, ah, al, bh, bl) \
  do {									\
    mp_limb_t __x;							\
    __x = (al) + (bl);							\
    (sh) = (ah) + (bh) + (__x < (al));					\
    (sl) = __x;								\
  } while (0)

#define gmp_sub_ddmmss(sh, sl, ah, al, bh, bl) \
  do {									\
    mp_limb_t __x;							\
    __x = (al) - (bl);							\
    (sh) = (ah) - (bh) - ((al) < (bl));					\
    (sl) = __x;								\
  } while (0)

#define gmp_umul_ppmm(w1, w0, u, v)					\
  do {									\
    mp_limb_t __x0, __x1, __x2, __x3;					\
    unsigned __ul, __vl, __uh, __vh;					\
    mp_limb_t __u = (u), __v = (v);					\
									\
    __ul = __u & GMP_LLIMB_MASK;					\
    __uh = __u >> (GMP_LIMB_BITS / 2);					\
    __vl = __v & GMP_LLIMB_MASK;					\
    __vh = __v >> (GMP_LIMB_BITS / 2);					\
									\
    __x0 = (mp_limb_t) __ul * __vl;					\
    __x1 = (mp_limb_t) __ul * __vh;					\
    __x2 = (mp_limb_t) __uh * __vl;					\
    __x3 = (mp_limb_t) __uh * __vh;					\
									\
    __x1 += __x0 >> (GMP_LIMB_BITS / 2);/* this can't give carry */	\
    __x1 += __x2;		/* but this indeed can */		\
    if (__x1 < __x2)		/* did we get it? */			\
      __x3 += GMP_HLIMB_BIT;	/* yes, add it in the proper pos. */	\
									\
    (w1) = __x3 + (__x1 >> (GMP_LIMB_BITS / 2));			\
    (w0) = (__x1 << (GMP_LIMB_BITS / 2)) + (__x0 & GMP_LLIMB_MASK);	\
  } while (0)

#define gmp_udiv_qrnnd_preinv(q, r, nh, nl, d, di)			\
  do {									\
    mp_limb_t _qh, _ql, _r, _mask;					\
    gmp_umul_ppmm (_qh, _ql, (nh), (di));				\
    gmp_add_ssaaaa (_qh, _ql, _qh, _ql, (nh) + 1, (nl));		\
    _r = (nl) - _qh * (d);						\
    _mask = -(mp_limb_t) (_r > _ql); /* both > and >= are OK */		\
    _qh += _mask;							\
    _r += _mask & (d);							\
    if (_r >= (d))							\
      {									\
	_r -= (d);							\
	_qh++;								\
      }									\
									\
    (r) = _r;								\
    (q) = _qh;								\
  } while (0)

#define gmp_udiv_qr_3by2(q, r1, r0, n2, n1, n0, d1, d0, dinv)		\
  do {									\
    mp_limb_t _q0, _t1, _t0, _mask;					\
    gmp_umul_ppmm ((q), _q0, (n2), (dinv));				\
    gmp_add_ssaaaa ((q), _q0, (q), _q0, (n2), (n1));			\
									\
    /* Compute the two most significant limbs of n - q'd */		\
    (r1) = (n1) - (d1) * (q);						\
    gmp_sub_ddmmss ((r1), (r0), (r1), (n0), (d1), (d0));		\
    gmp_umul_ppmm (_t1, _t0, (d0), (q));				\
    gmp_sub_ddmmss ((r1), (r0), (r1), (r0), _t1, _t0);			\
    (q)++;								\
									\
    /* Conditionally adjust q and the remainders */			\
    _mask = - (mp_limb_t) ((r1) >= _q0);				\
    (q) += _mask;							\
    gmp_add_ssaaaa ((r1), (r0), (r1), (r0), _mask & (d1), _mask & (d0)); \
    if ((r1) >= (d1))							\
      {									\
	if ((r1) > (d1) || (r0) >= (d0))				\
	  {								\
	    (q)++;							\
	    gmp_sub_ddmmss ((r1), (r0), (r1), (r0), (d1), (d0));	\
	  }								\
      }									\
  } while (0)

/* Swap macros. */
#define MP_LIMB_T_SWAP(x, y)						\
  do {									\
    mp_limb_t __mp_limb_t_swap__tmp = (x);				\
    (x) = (y);								\
    (y) = __mp_limb_t_swap__tmp;					\
  } while (0)
#define MP_SIZE_T_SWAP(x, y)						\
  do {									\
    mp_size_t __mp_size_t_swap__tmp = (x);				\
    (x) = (y);								\
    (y) = __mp_size_t_swap__tmp;					\
  } while (0)
#define MP_BITCNT_T_SWAP(x,y)			\
  do {						\
    mp_bitcnt_t __mp_bitcnt_t_swap__tmp = (x);	\
    (x) = (y);					\
    (y) = __mp_bitcnt_t_swap__tmp;		\
  } while (0)
#define MP_PTR_SWAP(x, y)						\
  do {									\
    mp_ptr __mp_ptr_swap__tmp = (x);					\
    (x) = (y);								\
    (y) = __mp_ptr_swap__tmp;						\
  } while (0)
#define MP_SRCPTR_SWAP(x, y)						\
  do {									\
    mp_srcptr __mp_srcptr_swap__tmp = (x);				\
    (x) = (y);								\
    (y) = __mp_srcptr_swap__tmp;					\
  } while (0)

#define MPN_PTR_SWAP(xp,xs, yp,ys)					\
  do {									\
    MP_PTR_SWAP (xp, yp);						\
    MP_SIZE_T_SWAP (xs, ys);						\
  } while(0)
#define MPN_SRCPTR_SWAP(xp,xs, yp,ys)					\
  do {									\
    MP_SRCPTR_SWAP (xp, yp);						\
    MP_SIZE_T_SWAP (xs, ys);						\
  } while(0)

#define MPZ_PTR_SWAP(x, y)						\
  do {									\
    mpz_ptr __mpz_ptr_swap__tmp = (x);					\
    (x) = (y);								\
    (y) = __mpz_ptr_swap__tmp;						\
  } while (0)
#define MPZ_SRCPTR_SWAP(x, y)						\
  do {									\
    mpz_srcptr __mpz_srcptr_swap__tmp = (x);				\
    (x) = (y);								\
    (y) = __mpz_srcptr_swap__tmp;					\
  } while (0)

static const int mp_bits_per_limb = GMP_LIMB_BITS;

/* Memory allocation and other helper functions. */
static void
gmp_die (const char *msg)
{
  fprintf (stderr, "%s\n", msg);
  abort();
}

static void *
gmp_default_alloc (size_t size)
{
  void *p;

  assert (size > 0);

  p = malloc (size);
  if (!p)
    gmp_die("gmp_default_alloc: Virtual memory exhausted.");

  return p;
}

static void *
gmp_default_realloc (void *old, size_t old_size, size_t new_size)
{
  void * p;

  p = realloc (old, new_size);

  if (!p)
    gmp_die("gmp_default_realloc: Virtual memory exhausted.");

  return p;
}

static void
gmp_default_free (void *p, size_t size)
{
  free (p);
}

static void * (*gmp_allocate_func) (size_t) = gmp_default_alloc;
static void * (*gmp_reallocate_func) (void *, size_t, size_t) = gmp_default_realloc;
static void (*gmp_free_func) (void *, size_t) = gmp_default_free;

#define gmp_xalloc(size) ((*gmp_allocate_func)((size)))
#define gmp_free(p) ((*gmp_free_func) ((p), 0))

static mp_ptr
gmp_xalloc_limbs (mp_size_t size)
{
  return (mp_ptr) gmp_xalloc (size * sizeof (mp_limb_t));
}

static mp_ptr
gmp_xrealloc_limbs (mp_ptr old, mp_size_t size)
{
  assert (size > 0);
  return (mp_ptr) (*gmp_reallocate_func) (old, 0, size * sizeof (mp_limb_t));
}

static mp_ptr
mpz_realloc (mpz_t r, mp_size_t size)
{
  size = GMP_MAX (size, 1);

  if (r->_mp_alloc)
    r->_mp_d = gmp_xrealloc_limbs (r->_mp_d, size);
  else
    r->_mp_d = gmp_xalloc_limbs (size);
  r->_mp_alloc = size;

  if (GMP_ABS (r->_mp_size) > size)
    r->_mp_size = 0;

  return r->_mp_d;
}

/* Realloc for an mpz_t WHAT if it has less than NEEDED limbs.  */
#define MPZ_REALLOC(z,n) ((n) > (z)->_mp_alloc			\
			  ? mpz_realloc(z,n)			\
			  : (z)->_mp_d)


static mp_size_t
mpn_normalized_size (mp_srcptr xp, mp_size_t n)
{
  while (n > 0 && xp[n-1] == 0)
    --n;
  return n;
}

static void
mpn_copyi (mp_ptr d, mp_srcptr s, mp_size_t n)
{
  mp_size_t i;
  for (i = 0; i < n; i++)
    d[i] = s[i];
}

static void
mpn_copyd (mp_ptr d, mp_srcptr s, mp_size_t n)
{
  while (--n >= 0)
    d[n] = s[n];
}

static int
mpn_zero_p(mp_srcptr rp, mp_size_t n)
{
  return mpn_normalized_size (rp, n) == 0;
}

static void
mpn_zero (mp_ptr rp, mp_size_t n)
{
  while (--n >= 0)
    rp[n] = 0;
}

void
mpn_com (mp_ptr rp, mp_srcptr up, mp_size_t n)
{
  while (--n >= 0)
    *rp++ = ~ *up++;
}

static mp_limb_t
mpn_neg (mp_ptr rp, mp_srcptr up, mp_size_t n)
{
  while (*up == 0)
    {
      *rp = 0;
      if (!--n)
	return 0;
      ++up; ++rp;
    }
  *rp = - *up;
  mpn_com (++rp, ++up, --n);
  return 1;
}

/* MPZ interface */
static void
mpz_init (mpz_t r)
{
  static const mp_limb_t dummy_limb = 0xc1a0;

  r->_mp_alloc = 0;
  r->_mp_size = 0;
  r->_mp_d = (mp_ptr) &dummy_limb;
}

/* The utility of this function is a bit limited, since many functions
   assigns the result variable using mpz_swap. */
void
mpz_init2 (mpz_t r, mp_bitcnt_t bits)
{
  mp_size_t rn;

  bits -= (bits != 0);		/* Round down, except if 0 */
  rn = 1 + bits / GMP_LIMB_BITS;

  r->_mp_alloc = rn;
  r->_mp_size = 0;
  r->_mp_d = gmp_xalloc_limbs (rn);
}

static void
mpz_clear (mpz_t r)
{
  if (r->_mp_alloc)
    gmp_free (r->_mp_d);
}

/* MPZ assignment and basic conversions. */

static void
mpz_set_ui (mpz_t r, unsigned long int x)
{
  if (x > 0)
    {
      r->_mp_size = 1;
      MPZ_REALLOC (r, 1)[0] = x;
    }
  else
    r->_mp_size = 0;
}

static void
mpz_set_si (mpz_t r, signed long int x)
{
  if (x >= 0)
    mpz_set_ui (r, x);
  else /* (x < 0) */
    {
      r->_mp_size = -1;
      MPZ_REALLOC (r, 1)[0] = GMP_NEG_CAST (unsigned long int, x);
    }
}

static void
mpz_set (mpz_t r, const mpz_t x)
{
  /* Allow the NOP r == x */
  if (r != x)
    {
      mp_size_t n;
      mp_ptr rp;

      n = GMP_ABS (x->_mp_size);
      rp = MPZ_REALLOC (r, n);

      mpn_copyi (rp, x->_mp_d, n);
      r->_mp_size = x->_mp_size;
    }
}

static void
mpz_init_set_si (mpz_t r, signed long int x)
{
  mpz_init (r);
  mpz_set_si (r, x);
}

static void
mpz_init_set_ui (mpz_t r, unsigned long int x)
{
  mpz_init (r);
  mpz_set_ui (r, x);
}

static void
mpz_init_set (mpz_t r, const mpz_t x)
{
  mpz_init (r);
  mpz_set (r, x);
}

static int
mpz_sgn (const mpz_t u)
{
  return GMP_CMP (u->_mp_size, 0);
}

static unsigned long int
mpz_get_ui (const mpz_t u)
{
  return u->_mp_size == 0 ? 0 : u->_mp_d[0];
}

static long int
mpz_get_si (const mpz_t u)
{
  if (u->_mp_size < 0)
    /* This expression is necessary to properly handle 0x80000000 */
    return -1 - (long) ((u->_mp_d[0] - 1) & ~GMP_LIMB_HIGHBIT);
  else
    return (long) (mpz_get_ui (u) & ~GMP_LIMB_HIGHBIT);
}

static int
mpn_cmp (mp_srcptr ap, mp_srcptr bp, mp_size_t n)
{
  while (--n >= 0)
    {
      if (ap[n] != bp[n])
	return ap[n] > bp[n] ? 1 : -1;
    }
  return 0;
}

static int
mpn_cmp4 (mp_srcptr ap, mp_size_t an, mp_srcptr bp, mp_size_t bn)
{
  if (an != bn)
    return an < bn ? -1 : 1;
  else
    return mpn_cmp (ap, bp, an);
}

static int
mpz_cmp_ui (const mpz_t u, unsigned long v)
{
  mp_size_t usize = u->_mp_size;

  if (usize > 1)
    return 1;
  else if (usize < 0)
    return -1;
  else
    return GMP_CMP (mpz_get_ui (u), v);
}

static int
mpz_cmp_si (const mpz_t u, long v)
{
  mp_size_t usize = u->_mp_size;

  if (usize < -1)
    return -1;
  else if (v >= 0)
    return mpz_cmp_ui (u, v);
  else if (usize >= 0)
    return 1;
  else /* usize == -1 */
    return GMP_CMP (GMP_NEG_CAST (mp_limb_t, v), u->_mp_d[0]);
}

static int
mpz_cmp (const mpz_t a, const mpz_t b)
{
  mp_size_t asize = a->_mp_size;
  mp_size_t bsize = b->_mp_size;

  if (asize != bsize)
    return (asize < bsize) ? -1 : 1;
  else if (asize >= 0)
    return mpn_cmp (a->_mp_d, b->_mp_d, asize);
  else
    return mpn_cmp (b->_mp_d, a->_mp_d, -asize);
}

static int
mpz_cmpabs_ui (const mpz_t u, unsigned long v)
{
  if (GMP_ABS (u->_mp_size) > 1)
    return 1;
  else
    return GMP_CMP (mpz_get_ui (u), v);
}

static void
mpz_abs (mpz_t r, const mpz_t u)
{
  mpz_set (r, u);
  r->_mp_size = GMP_ABS (r->_mp_size);
}

static void
mpz_neg (mpz_t r, const mpz_t u)
{
  mpz_set (r, u);
  r->_mp_size = -r->_mp_size;
}

static void
mpz_swap (mpz_t u, mpz_t v)
{
  MP_SIZE_T_SWAP (u->_mp_size, v->_mp_size);
  MP_SIZE_T_SWAP (u->_mp_alloc, v->_mp_alloc);
  MP_PTR_SWAP (u->_mp_d, v->_mp_d);
}

static mp_limb_t
mpn_add_1 (mp_ptr rp, mp_srcptr ap, mp_size_t n, mp_limb_t b)
{
  mp_size_t i;

  assert (n > 0);
  i = 0;
  do
    {
      mp_limb_t r = ap[i] + b;
      /* Carry out */
      b = (r < b);
      rp[i] = r;
    }
  while (++i < n);

  return b;
}

static mp_limb_t
mpn_add_n (mp_ptr rp, mp_srcptr ap, mp_srcptr bp, mp_size_t n)
{
  mp_size_t i;
  mp_limb_t cy;

  for (i = 0, cy = 0; i < n; i++)
    {
      mp_limb_t a, b, r;
      a = ap[i]; b = bp[i];
      r = a + cy;
      cy = (r < cy);
      r += b;
      cy += (r < b);
      rp[i] = r;
    }
  return cy;
}

static mp_limb_t
mpn_sub_1 (mp_ptr rp, mp_srcptr ap, mp_size_t n, mp_limb_t b)
{
  mp_size_t i;

  assert (n > 0);

  i = 0;
  do
    {
      mp_limb_t a = ap[i];
      /* Carry out */
      mp_limb_t cy = a < b;
      rp[i] = a - b;
      b = cy;
    }
  while (++i < n);

  return b;
}

static mp_limb_t
mpn_sub_n (mp_ptr rp, mp_srcptr ap, mp_srcptr bp, mp_size_t n)
{
  mp_size_t i;
  mp_limb_t cy;

  for (i = 0, cy = 0; i < n; i++)
    {
      mp_limb_t a, b;
      a = ap[i]; b = bp[i];
      b += cy;
      cy = (b < cy);
      cy += (a < b);
      rp[i] = a - b;
    }
  return cy;
}

static mp_limb_t
mpn_add (mp_ptr rp, mp_srcptr ap, mp_size_t an, mp_srcptr bp, mp_size_t bn)
{
  mp_limb_t cy;

  assert (an >= bn);

  cy = mpn_add_n (rp, ap, bp, bn);
  if (an > bn)
    cy = mpn_add_1 (rp + bn, ap + bn, an - bn, cy);
  return cy;
}

static mp_limb_t
mpn_sub (mp_ptr rp, mp_srcptr ap, mp_size_t an, mp_srcptr bp, mp_size_t bn)
{
  mp_limb_t cy;

  assert (an >= bn);

  cy = mpn_sub_n (rp, ap, bp, bn);
  if (an > bn)
    cy = mpn_sub_1 (rp + bn, ap + bn, an - bn, cy);
  return cy;
}

static mp_limb_t
mpn_mul_1 (mp_ptr rp, mp_srcptr up, mp_size_t n, mp_limb_t vl)
{
  mp_limb_t ul, cl, hpl, lpl;

  assert (n >= 1);

  cl = 0;
  do
    {
      ul = *up++;
      gmp_umul_ppmm (hpl, lpl, ul, vl);

      lpl += cl;
      cl = (lpl < cl) + hpl;

      *rp++ = lpl;
    }
  while (--n != 0);

  return cl;
}


static mp_limb_t
mpn_addmul_1 (mp_ptr rp, mp_srcptr up, mp_size_t n, mp_limb_t vl)
{
  mp_limb_t ul, cl, hpl, lpl, rl;

  assert (n >= 1);

  cl = 0;
  do
    {
      ul = *up++;
      gmp_umul_ppmm (hpl, lpl, ul, vl);

      lpl += cl;
      cl = (lpl < cl) + hpl;

      rl = *rp;
      lpl = rl + lpl;
      cl += lpl < rl;
      *rp++ = lpl;
    }
  while (--n != 0);

  return cl;
}

static mp_limb_t
mpn_submul_1 (mp_ptr rp, mp_srcptr up, mp_size_t n, mp_limb_t vl)
{
  mp_limb_t ul, cl, hpl, lpl, rl;

  assert (n >= 1);

  cl = 0;
  do
    {
      ul = *up++;
      gmp_umul_ppmm (hpl, lpl, ul, vl);

      lpl += cl;
      cl = (lpl < cl) + hpl;

      rl = *rp;
      lpl = rl - lpl;
      cl += lpl > rl;
      *rp++ = lpl;
    }
  while (--n != 0);

  return cl;
}

static mp_limb_t
mpn_mul (mp_ptr rp, mp_srcptr up, mp_size_t un, mp_srcptr vp, mp_size_t vn)
{
  assert (un >= vn);
  assert (vn >= 1);

  /* We first multiply by the low order limb. This result can be
     stored, not added, to rp. We also avoid a loop for zeroing this
     way. */

  rp[un] = mpn_mul_1 (rp, up, un, vp[0]);

  /* Now accumulate the product of up[] and the next higher limb from
     vp[]. */

  while (--vn >= 1)
    {
      rp += 1, vp += 1;
      rp[un] = mpn_addmul_1 (rp, up, un, vp[0]);
    }
  return rp[un];
}

static void
mpz_mul_ui (mpz_t r, const mpz_t u, unsigned long int v)
{
  mp_size_t un, us;
  mp_ptr tp;
  mp_limb_t cy;

  us = u->_mp_size;

  if (us == 0 || v == 0)
    {
      r->_mp_size = 0;
      return;
    }

  un = GMP_ABS (us);

  tp = MPZ_REALLOC (r, un + 1);
  cy = mpn_mul_1 (tp, u->_mp_d, un, v);
  tp[un] = cy;

  un += (cy > 0);
  r->_mp_size = (us < 0) ? - un : un;
}

static void
mpz_mul (mpz_t r, const mpz_t u, const mpz_t v)
{
  int sign;
  mp_size_t un, vn, rn;
  mpz_t t;
  mp_ptr tp;

  un = u->_mp_size;
  vn = v->_mp_size;

  if (un == 0 || vn == 0)
    {
      r->_mp_size = 0;
      return;
    }

  sign = (un ^ vn) < 0;

  un = GMP_ABS (un);
  vn = GMP_ABS (vn);

  mpz_init2 (t, (un + vn) * GMP_LIMB_BITS);

  tp = t->_mp_d;
  if (un >= vn)
    mpn_mul (tp, u->_mp_d, un, v->_mp_d, vn);
  else
    mpn_mul (tp, v->_mp_d, vn, u->_mp_d, un);

  rn = un + vn;
  rn -= tp[rn-1] == 0;

  t->_mp_size = sign ? - rn : rn;
  mpz_swap (r, t);
  mpz_clear (t);
}

/* Adds to the absolute value. Returns new size, but doesn't store it. */
static mp_size_t
mpz_abs_add_ui (mpz_t r, const mpz_t a, unsigned long b)
{
  mp_size_t an;
  mp_ptr rp;
  mp_limb_t cy;

  an = GMP_ABS (a->_mp_size);
  if (an == 0)
    {
      MPZ_REALLOC (r, 1)[0] = b;
      return b > 0;
    }

  rp = MPZ_REALLOC (r, an + 1);

  cy = mpn_add_1 (rp, a->_mp_d, an, b);
  rp[an] = cy;
  an += cy;

  return an;
}

/* Subtract from the absolute value. Returns new size, (or -1 on underflow),
   but doesn't store it. */
static mp_size_t
mpz_abs_sub_ui (mpz_t r, const mpz_t a, unsigned long b)
{
  mp_size_t an = GMP_ABS (a->_mp_size);
  mp_ptr rp;

  if (an == 0)
    {
      MPZ_REALLOC (r, 1)[0] = b;
      return -(b > 0);
    }
  rp = MPZ_REALLOC (r, an);
  if (an == 1 && a->_mp_d[0] < b)
    {
      rp[0] = b - a->_mp_d[0];
      return -1;
    }
  else
    {
      gmp_assert_nocarry (mpn_sub_1 (rp, a->_mp_d, an, b));
      return mpn_normalized_size (rp, an);
    }
}

static void
mpz_add_ui (mpz_t r, const mpz_t a, unsigned long b)
{
  if (a->_mp_size >= 0)
    r->_mp_size = mpz_abs_add_ui (r, a, b);
  else
    r->_mp_size = -mpz_abs_sub_ui (r, a, b);
}

static void
mpz_sub_ui (mpz_t r, const mpz_t a, unsigned long b)
{
  if (a->_mp_size < 0)
    r->_mp_size = -mpz_abs_add_ui (r, a, b);
  else
    r->_mp_size = mpz_abs_sub_ui (r, a, b);
}

static mp_size_t
mpz_abs_add (mpz_t r, const mpz_t a, const mpz_t b)
{
  mp_size_t an = GMP_ABS (a->_mp_size);
  mp_size_t bn = GMP_ABS (b->_mp_size);
  mp_ptr rp;
  mp_limb_t cy;

  if (an < bn)
    {
      MPZ_SRCPTR_SWAP (a, b);
      MP_SIZE_T_SWAP (an, bn);
    }

  rp = MPZ_REALLOC (r, an + 1);
  cy = mpn_add (rp, a->_mp_d, an, b->_mp_d, bn);

  rp[an] = cy;

  return an + cy;
}

static mp_size_t
mpz_abs_sub (mpz_t r, const mpz_t a, const mpz_t b)
{
  mp_size_t an = GMP_ABS (a->_mp_size);
  mp_size_t bn = GMP_ABS (b->_mp_size);
  int cmp;
  mp_ptr rp;

  cmp = mpn_cmp4 (a->_mp_d, an, b->_mp_d, bn);
  if (cmp > 0)
    {
      rp = MPZ_REALLOC (r, an);
      gmp_assert_nocarry (mpn_sub (rp, a->_mp_d, an, b->_mp_d, bn));
      return mpn_normalized_size (rp, an);
    }
  else if (cmp < 0)
    {
      rp = MPZ_REALLOC (r, bn);
      gmp_assert_nocarry (mpn_sub (rp, b->_mp_d, bn, a->_mp_d, an));
      return -mpn_normalized_size (rp, bn);
    }
  else
    return 0;
}

static void
mpz_add (mpz_t r, const mpz_t a, const mpz_t b)
{
  mp_size_t rn;

  if ( (a->_mp_size ^ b->_mp_size) >= 0)
    rn = mpz_abs_add (r, a, b);
  else
    rn = mpz_abs_sub (r, a, b);

  r->_mp_size = a->_mp_size >= 0 ? rn : - rn;
}

static void
mpz_sub (mpz_t r, const mpz_t a, const mpz_t b)
{
  mp_size_t rn;

  if ( (a->_mp_size ^ b->_mp_size) >= 0)
    rn = mpz_abs_sub (r, a, b);
  else
    rn = mpz_abs_add (r, a, b);

  r->_mp_size = a->_mp_size >= 0 ? rn : - rn;
}

static void
mpz_pow_ui (mpz_t r, const mpz_t b, unsigned long e)
{
  unsigned long bit;
  mpz_t tr;
  mpz_init_set_ui (tr, 1);

  bit = GMP_ULONG_HIGHBIT;
  do
    {
      mpz_mul (tr, tr, tr);
      if (e & bit)
	mpz_mul (tr, tr, b);
      bit >>= 1;
    }
  while (bit > 0);

  mpz_swap (r, tr);
  mpz_clear (tr);
}


static mp_limb_t
mpn_lshift (mp_ptr rp, mp_srcptr up, mp_size_t n, unsigned int cnt)
{
  mp_limb_t high_limb, low_limb;
  unsigned int tnc;
  mp_limb_t retval;

  assert (n >= 1);
  assert (cnt >= 1);
  assert (cnt < GMP_LIMB_BITS);

  up += n;
  rp += n;

  tnc = GMP_LIMB_BITS - cnt;
  low_limb = *--up;
  retval = low_limb >> tnc;
  high_limb = (low_limb << cnt);

  while (--n != 0)
    {
      low_limb = *--up;
      *--rp = high_limb | (low_limb >> tnc);
      high_limb = (low_limb << cnt);
    }
  *--rp = high_limb;

  return retval;
}

static mp_limb_t
mpn_rshift (mp_ptr rp, mp_srcptr up, mp_size_t n, unsigned int cnt)
{
  mp_limb_t high_limb, low_limb;
  unsigned int tnc;
  mp_limb_t retval;

  assert (n >= 1);
  assert (cnt >= 1);
  assert (cnt < GMP_LIMB_BITS);

  tnc = GMP_LIMB_BITS - cnt;
  high_limb = *up++;
  retval = (high_limb << tnc);
  low_limb = high_limb >> cnt;

  while (--n != 0)
    {
      high_limb = *up++;
      *rp++ = low_limb | (high_limb << tnc);
      low_limb = high_limb >> cnt;
    }
  *rp = low_limb;

  return retval;
}


/* MPN division interface. */

/* The 3/2 inverse is defined as

     m = floor( (B^3-1) / (B u1 + u0)) - B
*/
static mp_limb_t
mpn_invert_3by2 (mp_limb_t u1, mp_limb_t u0)
{
  mp_limb_t r, p, m, ql;
  unsigned ul, uh, qh;

  assert (u1 >= GMP_LIMB_HIGHBIT);

  /* For notation, let b denote the half-limb base, so that B = b^2.
     Split u1 = b uh + ul. */
  ul = u1 & GMP_LLIMB_MASK;
  uh = u1 >> (GMP_LIMB_BITS / 2);

  /* Approximation of the high half of quotient. Differs from the 2/1
     inverse of the half limb uh, since we have already subtracted
     u0. */
  qh = ~u1 / uh;

  /* Adjust to get a half-limb 3/2 inverse, i.e., we want

     qh' = floor( (b^3 - 1) / u) - b = floor ((b^3 - b u - 1) / u
         = floor( (b (~u) + b-1) / u),

     and the remainder

     r = b (~u) + b-1 - qh (b uh + ul)
       = b (~u - qh uh) + b-1 - qh ul

     Subtraction of qh ul may underflow, which implies adjustments.
     But by normalization, 2 u >= B > qh ul, so we need to adjust by
     at most 2.
  */

  r = ((~u1 - (mp_limb_t) qh * uh) << (GMP_LIMB_BITS / 2)) | GMP_LLIMB_MASK;

  p = (mp_limb_t) qh * ul;
  /* Adjustment steps taken from udiv_qrnnd_c */
  if (r < p)
    {
      qh--;
      r += u1;
      if (r >= u1) /* i.e. we didn't get carry when adding to r */
	if (r < p)
	  {
	    qh--;
	    r += u1;
	  }
    }
  r -= p;

  /* Low half of the quotient is

       ql = floor ( (b r + b-1) / u1).

     This is a 3/2 division (on half-limbs), for which qh is a
     suitable inverse. */

  p = (r >> (GMP_LIMB_BITS / 2)) * qh + r;
  /* Unlike full-limb 3/2, we can add 1 without overflow. For this to
     work, it is essential that ql is a full mp_limb_t. */
  ql = (p >> (GMP_LIMB_BITS / 2)) + 1;

  /* By the 3/2 trick, we don't need the high half limb. */
  r = (r << (GMP_LIMB_BITS / 2)) + GMP_LLIMB_MASK - ql * u1;

  if (r >= (p << (GMP_LIMB_BITS / 2)))
    {
      ql--;
      r += u1;
    }
  m = ((mp_limb_t) qh << (GMP_LIMB_BITS / 2)) + ql;
  if (r >= u1)
    {
      m++;
      r -= u1;
    }

  /* Now m is the 2/1 invers of u1. If u0 > 0, adjust it to become a
     3/2 inverse. */
  if (u0 > 0)
    {
      mp_limb_t th, tl;
      r = ~r;
      r += u0;
      if (r < u0)
	{
	  m--;
	  if (r >= u1)
	    {
	      m--;
	      r -= u1;
	    }
	  r -= u1;
	}
      gmp_umul_ppmm (th, tl, u0, m);
      r += th;
      if (r < th)
	{
	  m--;
	  m -= ((r > u1) | ((r == u1) & (tl > u0)));
	}
    }

  return m;
}
#define mpn_invert_limb(x) mpn_invert_3by2 ((x), 0)

static void
mpn_div_qr_1_invert (struct gmp_div_inverse *inv, mp_limb_t d)
{
  unsigned shift;

  assert (d > 0);
  gmp_clz (shift, d);
  inv->shift = shift;
  inv->d1 = d << shift;
  inv->di = mpn_invert_limb (inv->d1);
}

static void
mpn_div_qr_2_invert (struct gmp_div_inverse *inv,
		     mp_limb_t d1, mp_limb_t d0)
{
  unsigned shift;

  assert (d1 > 0);
  gmp_clz (shift, d1);
  inv->shift = shift;
  if (shift > 0)
    {
      d1 = (d1 << shift) | (d0 >> (GMP_LIMB_BITS - shift));
      d0 <<= shift;
    }
  inv->d1 = d1;
  inv->d0 = d0;
  inv->di = mpn_invert_3by2 (d1, d0);
}

static void
mpn_div_qr_invert (struct gmp_div_inverse *inv,
		   mp_srcptr dp, mp_size_t dn)
{
  assert (dn > 0);

  if (dn == 1)
    mpn_div_qr_1_invert (inv, dp[0]);
  else if (dn == 2)
    mpn_div_qr_2_invert (inv, dp[1], dp[0]);
  else
    {
      unsigned shift;
      mp_limb_t d1, d0;

      d1 = dp[dn-1];
      d0 = dp[dn-2];
      assert (d1 > 0);
      gmp_clz (shift, d1);
      inv->shift = shift;
      if (shift > 0)
	{
	  d1 = (d1 << shift) | (d0 >> (GMP_LIMB_BITS - shift));
	  d0 = (d0 << shift) | (dp[dn-3] >> (GMP_LIMB_BITS - shift));
	}
      inv->d1 = d1;
      inv->d0 = d0;
      inv->di = mpn_invert_3by2 (d1, d0);
    }
}

/* Not matching current public gmp interface, rather corresponding to
   the sbpi1_div_* functions. */
static mp_limb_t
mpn_div_qr_1_preinv (mp_ptr qp, mp_srcptr np, mp_size_t nn,
		     const struct gmp_div_inverse *inv)
{
  mp_limb_t d, di;
  mp_limb_t r;
  mp_ptr tp = NULL;

  if (inv->shift > 0)
    {
      tp = gmp_xalloc_limbs (nn);
      r = mpn_lshift (tp, np, nn, inv->shift);
      np = tp;
    }
  else
    r = 0;

  d = inv->d1;
  di = inv->di;
  while (--nn >= 0)
    {
      mp_limb_t q;

      gmp_udiv_qrnnd_preinv (q, r, r, np[nn], d, di);
      if (qp)
	qp[nn] = q;
    }
  if (inv->shift > 0)
    gmp_free (tp);

  return r >> inv->shift;
}

static mp_limb_t
mpn_div_qr_1 (mp_ptr qp, mp_srcptr np, mp_size_t nn, mp_limb_t d)
{
  assert (d > 0);

  /* Special case for powers of two. */
  if ((d & (d-1)) == 0)
    {
      mp_limb_t r = np[0] & (d-1);
      if (qp)
	{
	  if (d <= 1)
	    mpn_copyi (qp, np, nn);
	  else
	    {
	      unsigned shift;
	      gmp_ctz (shift, d);
	      mpn_rshift (qp, np, nn, shift);
	    }
	}
      return r;
    }
  else
    {
      struct gmp_div_inverse inv;
      mpn_div_qr_1_invert (&inv, d);
      return mpn_div_qr_1_preinv (qp, np, nn, &inv);
    }
}

static void
mpn_div_qr_2_preinv (mp_ptr qp, mp_ptr rp, mp_srcptr np, mp_size_t nn,
		     const struct gmp_div_inverse *inv)
{
  unsigned shift;
  mp_size_t i;
  mp_limb_t d1, d0, di, r1, r0;
  mp_ptr tp;

  assert (nn >= 2);
  shift = inv->shift;
  d1 = inv->d1;
  d0 = inv->d0;
  di = inv->di;

  if (shift > 0)
    {
      tp = gmp_xalloc_limbs (nn);
      r1 = mpn_lshift (tp, np, nn, shift);
      np = tp;
    }
  else
    r1 = 0;

  r0 = np[nn - 1];

  i = nn - 2;
  do
    {
      mp_limb_t n0, q;
      n0 = np[i];
      gmp_udiv_qr_3by2 (q, r1, r0, r1, r0, n0, d1, d0, di);

      if (qp)
	qp[i] = q;
    }
  while (--i >= 0);

  if (shift > 0)
    {
      assert ((r0 << (GMP_LIMB_BITS - shift)) == 0);
      r0 = (r0 >> shift) | (r1 << (GMP_LIMB_BITS - shift));
      r1 >>= shift;

      gmp_free (tp);
    }

  rp[1] = r1;
  rp[0] = r0;
}

static void
mpn_div_qr_pi1 (mp_ptr qp,
		mp_ptr np, mp_size_t nn, mp_limb_t n1,
		mp_srcptr dp, mp_size_t dn,
		mp_limb_t dinv)
{
  mp_size_t i;

  mp_limb_t d1, d0;
  mp_limb_t cy, cy1;
  mp_limb_t q;

  assert (dn > 2);
  assert (nn >= dn);

  d1 = dp[dn - 1];
  d0 = dp[dn - 2];

  assert ((d1 & GMP_LIMB_HIGHBIT) != 0);
  /* Iteration variable is the index of the q limb.
   *
   * We divide <n1, np[dn-1+i], np[dn-2+i], np[dn-3+i],..., np[i]>
   * by            <d1,          d0,        dp[dn-3],  ..., dp[0] >
   */

  i = nn - dn;
  do
    {
      mp_limb_t n0 = np[dn-1+i];

      if (n1 == d1 && n0 == d0)
	{
	  q = GMP_LIMB_MAX;
	  mpn_submul_1 (np+i, dp, dn, q);
	  n1 = np[dn-1+i];	/* update n1, last loop's value will now be invalid */
	}
      else
	{
	  gmp_udiv_qr_3by2 (q, n1, n0, n1, n0, np[dn-2+i], d1, d0, dinv);

	  cy = mpn_submul_1 (np + i, dp, dn-2, q);

	  cy1 = n0 < cy;
	  n0 = n0 - cy;
	  cy = n1 < cy1;
	  n1 = n1 - cy1;
	  np[dn-2+i] = n0;

	  if (cy != 0)
	    {
	      n1 += d1 + mpn_add_n (np + i, np + i, dp, dn - 1);
	      q--;
	    }
	}

      if (qp)
	qp[i] = q;
    }
  while (--i >= 0);

  np[dn - 1] = n1;
}

static void
mpn_div_qr_preinv (mp_ptr qp, mp_ptr np, mp_size_t nn,
		   mp_srcptr dp, mp_size_t dn,
		   const struct gmp_div_inverse *inv)
{
  assert (dn > 0);
  assert (nn >= dn);

  if (dn == 1)
    np[0] = mpn_div_qr_1_preinv (qp, np, nn, inv);
  else if (dn == 2)
    mpn_div_qr_2_preinv (qp, np, np, nn, inv);
  else
    {
      mp_limb_t nh;
      unsigned shift;

      assert (inv->d1 == dp[dn-1]);
      assert (inv->d0 == dp[dn-2]);
      assert ((inv->d1 & GMP_LIMB_HIGHBIT) != 0);

      shift = inv->shift;
      if (shift > 0)
	nh = mpn_lshift (np, np, nn, shift);
      else
	nh = 0;

      mpn_div_qr_pi1 (qp, np, nn, nh, dp, dn, inv->di);

      if (shift > 0)
	gmp_assert_nocarry (mpn_rshift (np, np, dn, shift));
    }
}


static mp_bitcnt_t
mpn_common_scan (mp_limb_t limb, mp_size_t i, mp_srcptr up, mp_size_t un,
		 mp_limb_t ux)
{
  unsigned cnt;

  assert (ux == 0 || ux == GMP_LIMB_MAX);
  assert (0 <= i && i <= un );

  while (limb == 0)
    {
      i++;
      if (i == un)
	return (ux == 0 ? ~(mp_bitcnt_t) 0 : un * GMP_LIMB_BITS);
      limb = ux ^ up[i];
    }
  gmp_ctz (cnt, limb);
  return (mp_bitcnt_t) i * GMP_LIMB_BITS + cnt;
}

/* MPZ division */
enum mpz_div_round_mode { GMP_DIV_FLOOR, GMP_DIV_CEIL, GMP_DIV_TRUNC };

static void
mpz_div_q_2exp (mpz_t q, const mpz_t u, mp_bitcnt_t bit_index,
		enum mpz_div_round_mode mode)
{
  mp_size_t un, qn;
  mp_size_t limb_cnt;
  mp_ptr qp;
  int adjust;

  un = u->_mp_size;
  if (un == 0)
    {
      q->_mp_size = 0;
      return;
    }
  limb_cnt = bit_index / GMP_LIMB_BITS;
  qn = GMP_ABS (un) - limb_cnt;
  bit_index %= GMP_LIMB_BITS;

  if (mode == ((un > 0) ? GMP_DIV_CEIL : GMP_DIV_FLOOR)) /* un != 0 here. */
    /* Note: Below, the final indexing at limb_cnt is valid because at
       that point we have qn > 0. */
    adjust = (qn <= 0
	      || !mpn_zero_p (u->_mp_d, limb_cnt)
	      || (u->_mp_d[limb_cnt]
		  & (((mp_limb_t) 1 << bit_index) - 1)));
  else
    adjust = 0;

  if (qn <= 0)
    qn = 0;
  else
    {
      qp = MPZ_REALLOC (q, qn);

      if (bit_index != 0)
	{
	  mpn_rshift (qp, u->_mp_d + limb_cnt, qn, bit_index);
	  qn -= qp[qn - 1] == 0;
	}
      else
	{
	  mpn_copyi (qp, u->_mp_d + limb_cnt, qn);
	}
    }

  q->_mp_size = qn;

  if (adjust)
    mpz_add_ui (q, q, 1);
  if (un < 0)
    mpz_neg (q, q);
}

static void
mpz_div_r_2exp (mpz_t r, const mpz_t u, mp_bitcnt_t bit_index,
		enum mpz_div_round_mode mode)
{
  mp_size_t us, un, rn;
  mp_ptr rp;
  mp_limb_t mask;

  us = u->_mp_size;
  if (us == 0 || bit_index == 0)
    {
      r->_mp_size = 0;
      return;
    }
  rn = (bit_index + GMP_LIMB_BITS - 1) / GMP_LIMB_BITS;
  assert (rn > 0);

  rp = MPZ_REALLOC (r, rn);
  un = GMP_ABS (us);

  mask = GMP_LIMB_MAX >> (rn * GMP_LIMB_BITS - bit_index);

  if (rn > un)
    {
      /* Quotient (with truncation) is zero, and remainder is
	 non-zero */
      if (mode == ((us > 0) ? GMP_DIV_CEIL : GMP_DIV_FLOOR)) /* us != 0 here. */
	{
	  /* Have to negate and sign extend. */
	  mp_size_t i;

	  gmp_assert_nocarry (! mpn_neg (rp, u->_mp_d, un));
	  for (i = un; i < rn - 1; i++)
	    rp[i] = GMP_LIMB_MAX;

	  rp[rn-1] = mask;
	  us = -us;
	}
      else
	{
	  /* Just copy */
	  if (r != u)
	    mpn_copyi (rp, u->_mp_d, un);

	  rn = un;
	}
    }
  else
    {
      if (r != u)
	mpn_copyi (rp, u->_mp_d, rn - 1);

      rp[rn-1] = u->_mp_d[rn-1] & mask;

      if (mode == ((us > 0) ? GMP_DIV_CEIL : GMP_DIV_FLOOR)) /* us != 0 here. */
	{
	  /* If r != 0, compute 2^{bit_count} - r. */
	  mpn_neg (rp, rp, rn);

	  rp[rn-1] &= mask;

	  /* us is not used for anything else, so we can modify it
	     here to indicate flipped sign. */
	  us = -us;
	}
    }
  rn = mpn_normalized_size (rp, rn);
  r->_mp_size = us < 0 ? -rn : rn;
}


static void
mpz_tdiv_q_2exp (mpz_t r, const mpz_t u, mp_bitcnt_t cnt)
{
  mpz_div_q_2exp (r, u, cnt, GMP_DIV_TRUNC);
}

static void
mpn_div_qr (mp_ptr qp, mp_ptr np, mp_size_t nn, mp_srcptr dp, mp_size_t dn)
{
  struct gmp_div_inverse inv;
  mp_ptr tp = NULL;

  assert (dn > 0);
  assert (nn >= dn);

  mpn_div_qr_invert (&inv, dp, dn);
  if (dn > 2 && inv.shift > 0)
    {
      tp = gmp_xalloc_limbs (dn);
      gmp_assert_nocarry (mpn_lshift (tp, dp, dn, inv.shift));
      dp = tp;
    }
  mpn_div_qr_preinv (qp, np, nn, dp, dn, &inv);
  if (tp)
    gmp_free (tp);
}


/* Allows q or r to be zero. Returns 1 iff remainder is non-zero. */
static int
mpz_div_qr (mpz_t q, mpz_t r,
	    const mpz_t n, const mpz_t d, enum mpz_div_round_mode mode)
{
  mp_size_t ns, ds, nn, dn, qs;
  ns = n->_mp_size;
  ds = d->_mp_size;

  if (ds == 0)
    gmp_die("mpz_div_qr: Divide by zero.");

  if (ns == 0)
    {
      if (q)
	q->_mp_size = 0;
      if (r)
	r->_mp_size = 0;
      return 0;
    }

  nn = GMP_ABS (ns);
  dn = GMP_ABS (ds);

  qs = ds ^ ns;

  if (nn < dn)
    {
      if (mode == GMP_DIV_CEIL && qs >= 0)
	{
	  /* q = 1, r = n - d */
	  if (r)
	    mpz_sub (r, n, d);
	  if (q)
	    mpz_set_ui (q, 1);
	}
      else if (mode == GMP_DIV_FLOOR && qs < 0)
	{
	  /* q = -1, r = n + d */
	  if (r)
	    mpz_add (r, n, d);
	  if (q)
	    mpz_set_si (q, -1);
	}
      else
	{
	  /* q = 0, r = d */
	  if (r)
	    mpz_set (r, n);
	  if (q)
	    q->_mp_size = 0;
	}
      return 1;
    }
  else
    {
      mp_ptr np, qp;
      mp_size_t qn, rn;
      mpz_t tq, tr;

      mpz_init_set (tr, n);
      np = tr->_mp_d;

      qn = nn - dn + 1;

      if (q)
	{
	  mpz_init2 (tq, qn * GMP_LIMB_BITS);
	  qp = tq->_mp_d;
	}
      else
	qp = NULL;

      mpn_div_qr (qp, np, nn, d->_mp_d, dn);

      if (qp)
	{
	  qn -= (qp[qn-1] == 0);

	  tq->_mp_size = qs < 0 ? -qn : qn;
	}
      rn = mpn_normalized_size (np, dn);
      tr->_mp_size = ns < 0 ? - rn : rn;

      if (mode == GMP_DIV_FLOOR && qs < 0 && rn != 0)
	{
	  if (q)
	    mpz_sub_ui (tq, tq, 1);
	  if (r)
	    mpz_add (tr, tr, d);
	}
      else if (mode == GMP_DIV_CEIL && qs >= 0 && rn != 0)
	{
	  if (q)
	    mpz_add_ui (tq, tq, 1);
	  if (r)
	    mpz_sub (tr, tr, d);
	}

      if (q)
	{
	  mpz_swap (tq, q);
	  mpz_clear (tq);
	}
      if (r)
	mpz_swap (tr, r);

      mpz_clear (tr);

      return rn != 0;
    }
}

static void
mpz_cdiv_qr (mpz_t q, mpz_t r, const mpz_t n, const mpz_t d)
{
  mpz_div_qr (q, r, n, d, GMP_DIV_CEIL);
}

static void
mpz_fdiv_qr (mpz_t q, mpz_t r, const mpz_t n, const mpz_t d)
{
  mpz_div_qr (q, r, n, d, GMP_DIV_FLOOR);
}

static void
mpz_tdiv_qr (mpz_t q, mpz_t r, const mpz_t n, const mpz_t d)
{
  mpz_div_qr (q, r, n, d, GMP_DIV_TRUNC);
}

static void
mpz_cdiv_q (mpz_t q, const mpz_t n, const mpz_t d)
{
  mpz_div_qr (q, NULL, n, d, GMP_DIV_CEIL);
}

static void
mpz_fdiv_q (mpz_t q, const mpz_t n, const mpz_t d)
{
  mpz_div_qr (q, NULL, n, d, GMP_DIV_FLOOR);
}

static void
mpz_tdiv_q (mpz_t q, const mpz_t n, const mpz_t d)
{
  mpz_div_qr (q, NULL, n, d, GMP_DIV_TRUNC);
}

static void
mpz_cdiv_r (mpz_t r, const mpz_t n, const mpz_t d)
{
  mpz_div_qr (NULL, r, n, d, GMP_DIV_CEIL);
}

static void
mpz_fdiv_r (mpz_t r, const mpz_t n, const mpz_t d)
{
  mpz_div_qr (NULL, r, n, d, GMP_DIV_FLOOR);
}

static void
mpz_tdiv_r (mpz_t r, const mpz_t n, const mpz_t d)
{
  mpz_div_qr (NULL, r, n, d, GMP_DIV_TRUNC);
}

static void
mpz_divexact (mpz_t q, const mpz_t n, const mpz_t d)
{
  gmp_assert_nocarry (mpz_div_qr (q, NULL, n, d, GMP_DIV_TRUNC));
}

static unsigned long
mpz_div_qr_ui (mpz_t q, mpz_t r,
	       const mpz_t n, unsigned long d, enum mpz_div_round_mode mode)
{
  mp_size_t ns, qn;
  mp_ptr qp;
  mp_limb_t rl;
  mp_size_t rs;

  ns = n->_mp_size;
  if (ns == 0)
    {
      if (q)
	q->_mp_size = 0;
      if (r)
	r->_mp_size = 0;
      return 0;
    }

  qn = GMP_ABS (ns);
  if (q)
    qp = MPZ_REALLOC (q, qn);
  else
    qp = NULL;

  rl = mpn_div_qr_1 (qp, n->_mp_d, qn, d);
  assert (rl < d);

  rs = rl > 0;
  rs = (ns < 0) ? -rs : rs;

  if (rl > 0 && ( (mode == GMP_DIV_FLOOR && ns < 0)
		  || (mode == GMP_DIV_CEIL && ns >= 0)))
    {
      if (q)
	gmp_assert_nocarry (mpn_add_1 (qp, qp, qn, 1));
      rl = d - rl;
      rs = -rs;
    }

  if (r)
    {
      MPZ_REALLOC (r, 1)[0] = rl;
      r->_mp_size = rs;
    }
  if (q)
    {
      qn -= (qp[qn-1] == 0);
      assert (qn == 0 || qp[qn-1] > 0);

      q->_mp_size = (ns < 0) ? - qn : qn;
    }

  return rl;
}


static void
mpz_divexact_ui (mpz_t q, const mpz_t n, unsigned long d)
{
  gmp_assert_nocarry (mpz_div_qr_ui (q, NULL, n, d, GMP_DIV_TRUNC));
}


static mp_bitcnt_t
mpz_make_odd (mpz_t r)
{
  mp_bitcnt_t shift;

  assert (r->_mp_size > 0);
  /* Count trailing zeros, equivalent to mpn_scan1, because we know that there is a 1 */
  shift = mpn_common_scan (r->_mp_d[0], 0, r->_mp_d, 0, 0);
  mpz_tdiv_q_2exp (r, r, shift);

  return shift;
}

static void
mpn_sqr (mp_ptr rp, mp_srcptr ap, mp_size_t n)
{
  mpn_mul (rp, ap, n, ap, n);
}

static void
mpz_mul_2exp (mpz_t r, const mpz_t u, mp_bitcnt_t bits)
{
  mp_size_t un, rn;
  mp_size_t limbs;
  unsigned shift;
  mp_ptr rp;

  un = GMP_ABS (u->_mp_size);
  if (un == 0)
    {
      r->_mp_size = 0;
      return;
    }

  limbs = bits / GMP_LIMB_BITS;
  shift = bits % GMP_LIMB_BITS;

  rn = un + limbs + (shift > 0);
  rp = MPZ_REALLOC (r, rn);
  if (shift > 0)
    {
      mp_limb_t cy = mpn_lshift (rp + limbs, u->_mp_d, un, shift);
      rp[rn-1] = cy;
      rn -= (cy == 0);
    }
  else
    mpn_copyd (rp + limbs, u->_mp_d, un);

  mpn_zero (rp, limbs);

  r->_mp_size = (u->_mp_size < 0) ? - rn : rn;
}

static int
mpz_cmpabs (const mpz_t u, const mpz_t v)
{
  return mpn_cmp4 (u->_mp_d, GMP_ABS (u->_mp_size),
		   v->_mp_d, GMP_ABS (v->_mp_size));
}

#define mpz_odd_p(z)   (((z)->_mp_size != 0) & (int) (z)->_mp_d[0])
#define mpz_even_p(z)  (! mpz_odd_p (z))

static int
mpz_tstbit (const mpz_t d, mp_bitcnt_t bit_index)
{
  mp_size_t limb_index;
  unsigned shift;
  mp_size_t ds;
  mp_size_t dn;
  mp_limb_t w;
  int bit;

  ds = d->_mp_size;
  dn = GMP_ABS (ds);
  limb_index = bit_index / GMP_LIMB_BITS;
  if (limb_index >= dn)
    return ds < 0;

  shift = bit_index % GMP_LIMB_BITS;
  w = d->_mp_d[limb_index];
  bit = (w >> shift) & 1;

  if (ds < 0)
    {
      /* d < 0. Check if any of the bits below is set: If so, our bit
	 must be complemented. */
      if (shift > 0 && (w << (GMP_LIMB_BITS - shift)) > 0)
	return bit ^ 1;
      while (--limb_index >= 0)
	if (d->_mp_d[limb_index] > 0)
	  return bit ^ 1;
    }
  return bit;
}

static void
mpz_abs_add_bit (mpz_t d, mp_bitcnt_t bit_index)
{
  mp_size_t dn, limb_index;
  mp_limb_t bit;
  mp_ptr dp;

  dn = GMP_ABS (d->_mp_size);

  limb_index = bit_index / GMP_LIMB_BITS;
  bit = (mp_limb_t) 1 << (bit_index % GMP_LIMB_BITS);

  if (limb_index >= dn)
    {
      mp_size_t i;
      /* The bit should be set outside of the end of the number.
	 We have to increase the size of the number. */
      dp = MPZ_REALLOC (d, limb_index + 1);

      dp[limb_index] = bit;
      for (i = dn; i < limb_index; i++)
	dp[i] = 0;
      dn = limb_index + 1;
    }
  else
    {
      mp_limb_t cy;

      dp = d->_mp_d;

      cy = mpn_add_1 (dp + limb_index, dp + limb_index, dn - limb_index, bit);
      if (cy > 0)
	{
	  dp = MPZ_REALLOC (d, dn + 1);
	  dp[dn++] = cy;
	}
    }

  d->_mp_size = (d->_mp_size < 0) ? - dn : dn;
}

static void
mpz_abs_sub_bit (mpz_t d, mp_bitcnt_t bit_index)
{
  mp_size_t dn, limb_index;
  mp_ptr dp;
  mp_limb_t bit;

  dn = GMP_ABS (d->_mp_size);
  dp = d->_mp_d;

  limb_index = bit_index / GMP_LIMB_BITS;
  bit = (mp_limb_t) 1 << (bit_index % GMP_LIMB_BITS);

  assert (limb_index < dn);

  gmp_assert_nocarry (mpn_sub_1 (dp + limb_index, dp + limb_index,
				 dn - limb_index, bit));
  dn = mpn_normalized_size (dp, dn);
  d->_mp_size = (d->_mp_size < 0) ? - dn : dn;
}

static void
mpz_setbit (mpz_t d, mp_bitcnt_t bit_index)
{
  if (!mpz_tstbit (d, bit_index))
    {
      if (d->_mp_size >= 0)
	mpz_abs_add_bit (d, bit_index);
      else
	mpz_abs_sub_bit (d, bit_index);
    }
}

static void
mpz_gcdext (mpz_t g, mpz_t s, mpz_t t, const mpz_t u, const mpz_t v)
{
  mpz_t tu, tv, s0, s1, t0, t1;
  mp_bitcnt_t uz, vz, gz;
  mp_bitcnt_t power;

  if (u->_mp_size == 0)
    {
      /* g = 0 u + sgn(v) v */
      signed long sign = mpz_sgn (v);
      mpz_abs (g, v);
      if (s)
	mpz_set_ui (s, 0);
      if (t)
	mpz_set_si (t, sign);
      return;
    }

  if (v->_mp_size == 0)
    {
      /* g = sgn(u) u + 0 v */
      signed long sign = mpz_sgn (u);
      mpz_abs (g, u);
      if (s)
	mpz_set_si (s, sign);
      if (t)
	mpz_set_ui (t, 0);
      return;
    }

  mpz_init (tu);
  mpz_init (tv);
  mpz_init (s0);
  mpz_init (s1);
  mpz_init (t0);
  mpz_init (t1);

  mpz_abs (tu, u);
  uz = mpz_make_odd (tu);
  mpz_abs (tv, v);
  vz = mpz_make_odd (tv);
  gz = GMP_MIN (uz, vz);

  uz -= gz;
  vz -= gz;

  /* Cofactors corresponding to odd gcd. gz handled later. */
  if (tu->_mp_size < tv->_mp_size)
    {
      mpz_swap (tu, tv);
      MPZ_SRCPTR_SWAP (u, v);
      MPZ_PTR_SWAP (s, t);
      MP_BITCNT_T_SWAP (uz, vz);
    }

  /* Maintain
   *
   * u = t0 tu + t1 tv
   * v = s0 tu + s1 tv
   *
   * where u and v denote the inputs with common factors of two
   * eliminated, and det (s0, t0; s1, t1) = 2^p. Then
   *
   * 2^p tu =  s1 u - t1 v
   * 2^p tv = -s0 u + t0 v
   */

  /* After initial division, tu = q tv + tu', we have
   *
   * u = 2^uz (tu' + q tv)
   * v = 2^vz tv
   *
   * or
   *
   * t0 = 2^uz, t1 = 2^uz q
   * s0 = 0,    s1 = 2^vz
   */

  mpz_setbit (t0, uz);
  mpz_tdiv_qr (t1, tu, tu, tv);
  mpz_mul_2exp (t1, t1, uz);

  mpz_setbit (s1, vz);
  power = uz + vz;

  if (tu->_mp_size > 0)
    {
      mp_bitcnt_t shift;
      shift = mpz_make_odd (tu);
      mpz_mul_2exp (t0, t0, shift);
      mpz_mul_2exp (s0, s0, shift);
      power += shift;

      for (;;)
	{
	  int c;
	  c = mpz_cmp (tu, tv);
	  if (c == 0)
	    break;

	  if (c < 0)
	    {
	      /* tv = tv' + tu
	       *
	       * u = t0 tu + t1 (tv' + tu) = (t0 + t1) tu + t1 tv'
	       * v = s0 tu + s1 (tv' + tu) = (s0 + s1) tu + s1 tv' */

	      mpz_sub (tv, tv, tu);
	      mpz_add (t0, t0, t1);
	      mpz_add (s0, s0, s1);

	      shift = mpz_make_odd (tv);
	      mpz_mul_2exp (t1, t1, shift);
	      mpz_mul_2exp (s1, s1, shift);
	    }
	  else
	    {
	      mpz_sub (tu, tu, tv);
	      mpz_add (t1, t0, t1);
	      mpz_add (s1, s0, s1);

	      shift = mpz_make_odd (tu);
	      mpz_mul_2exp (t0, t0, shift);
	      mpz_mul_2exp (s0, s0, shift);
	    }
	  power += shift;
	}
    }

  /* Now tv = odd part of gcd, and -s0 and t0 are corresponding
     cofactors. */

  mpz_mul_2exp (tv, tv, gz);
  mpz_neg (s0, s0);

  /* 2^p g = s0 u + t0 v. Eliminate one factor of two at a time. To
     adjust cofactors, we need u / g and v / g */

  mpz_divexact (s1, v, tv);
  mpz_abs (s1, s1);
  mpz_divexact (t1, u, tv);
  mpz_abs (t1, t1);

  while (power-- > 0)
    {
      /* s0 u + t0 v = (s0 - v/g) u - (t0 + u/g) v */
      if (mpz_odd_p (s0) || mpz_odd_p (t0))
	{
	  mpz_sub (s0, s0, s1);
	  mpz_add (t0, t0, t1);
	}
      mpz_divexact_ui (s0, s0, 2);
      mpz_divexact_ui (t0, t0, 2);
    }

  /* Arrange so that |s| < |u| / 2g */
  mpz_add (s1, s0, s1);
  if (mpz_cmpabs (s0, s1) > 0)
    {
      mpz_swap (s0, s1);
      mpz_sub (t0, t0, t1);
    }
  if (u->_mp_size < 0)
    mpz_neg (s0, s0);
  if (v->_mp_size < 0)
    mpz_neg (t0, t0);

  mpz_swap (g, tv);
  if (s)
    mpz_swap (s, s0);
  if (t)
    mpz_swap (t, t0);

  mpz_clear (tu);
  mpz_clear (tv);
  mpz_clear (s0);
  mpz_clear (s1);
  mpz_clear (t0);
  mpz_clear (t1);
}


static int
mpz_invert (mpz_t r, const mpz_t u, const mpz_t m)
{
  mpz_t g, tr;
  int invertible;

  if (u->_mp_size == 0 || mpz_cmpabs_ui (m, 1) <= 0)
    return 0;

  mpz_init (g);
  mpz_init (tr);

  mpz_gcdext (g, tr, NULL, u, m);
  invertible = (mpz_cmp_ui (g, 1) == 0);

  if (invertible)
    {
      if (tr->_mp_size < 0)
	{
	  if (m->_mp_size >= 0)
	    mpz_add (tr, tr, m);
	  else
	    mpz_sub (tr, tr, m);
	}
      mpz_swap (r, tr);
    }

  mpz_clear (g);
  mpz_clear (tr);
  return invertible;
}


static void
mpz_powm (mpz_t r, const mpz_t b, const mpz_t e, const mpz_t m)
{
  mpz_t tr;
  mpz_t base;
  mp_size_t en, mn;
  mp_srcptr mp;
  struct gmp_div_inverse minv;
  unsigned shift;
  mp_ptr tp = NULL;

  en = GMP_ABS (e->_mp_size);
  mn = GMP_ABS (m->_mp_size);
  if (mn == 0)
    gmp_die ("mpz_powm: Zero modulo.");

  if (en == 0)
    {
      mpz_set_ui (r, 1);
      return;
    }

  mp = m->_mp_d;
  mpn_div_qr_invert (&minv, mp, mn);
  shift = minv.shift;

  if (shift > 0)
    {
      /* To avoid shifts, we do all our reductions, except the final
	 one, using a *normalized* m. */
      minv.shift = 0;

      tp = gmp_xalloc_limbs (mn);
      gmp_assert_nocarry (mpn_lshift (tp, mp, mn, shift));
      mp = tp;
    }

  mpz_init (base);

  if (e->_mp_size < 0)
    {
      if (!mpz_invert (base, b, m))
	gmp_die ("mpz_powm: Negative exponent and non-invertible base.");
    }
  else
    {
      mp_size_t bn;
      mpz_abs (base, b);

      bn = base->_mp_size;
      if (bn >= mn)
	{
	  mpn_div_qr_preinv (NULL, base->_mp_d, base->_mp_size, mp, mn, &minv);
	  bn = mn;
	}

      /* We have reduced the absolute value. Now take care of the
	 sign. Note that we get zero represented non-canonically as
	 m. */
      if (b->_mp_size < 0)
	{
	  mp_ptr bp = MPZ_REALLOC (base, mn);
	  gmp_assert_nocarry (mpn_sub (bp, mp, mn, bp, bn));
	  bn = mn;
	}
      base->_mp_size = mpn_normalized_size (base->_mp_d, bn);
    }
  mpz_init_set_ui (tr, 1);

  while (--en >= 0)
    {
      mp_limb_t w = e->_mp_d[en];
      mp_limb_t bit;

      bit = GMP_LIMB_HIGHBIT;
      do
	{
	  mpz_mul (tr, tr, tr);
	  if (w & bit)
	    mpz_mul (tr, tr, base);
	  if (tr->_mp_size > mn)
	    {
	      mpn_div_qr_preinv (NULL, tr->_mp_d, tr->_mp_size, mp, mn, &minv);
	      tr->_mp_size = mpn_normalized_size (tr->_mp_d, mn);
	    }
	  bit >>= 1;
	}
      while (bit > 0);
    }

  /* Final reduction */
  if (tr->_mp_size >= mn)
    {
      minv.shift = shift;
      mpn_div_qr_preinv (NULL, tr->_mp_d, tr->_mp_size, mp, mn, &minv);
      tr->_mp_size = mpn_normalized_size (tr->_mp_d, mn);
    }
  if (tp)
    gmp_free (tp);

  mpz_swap (r, tr);
  mpz_clear (tr);
  mpz_clear (base);
}