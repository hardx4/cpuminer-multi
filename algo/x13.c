#include "miner.h"

#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>

#include "sha3/sph_blake.h"
#include "sha3/sph_bmw.h"
#include "sha3/sph_groestl.h"
#include "sha3/sph_jh.h"
#include "sha3/sph_keccak.h"
#include "sha3/sph_skein.h"
#include "sha3/sph_sm3.h"
#include "sha3/sph_cubehash.h"
#include "sha3/sph_shavite.h"
#include "sha3/sph_simd.h"
#include "sha3/sph_echo.h"
#include "sha3/sph_hamsi.h"
#include "sha3/sph_fugue.h"

void x13hash(void *output, const void *input)
{
	unsigned char hash[128];

	memset(hash, 0, 128);

	sph_blake512_context     ctx_blake;
	sph_bmw512_context       ctx_bmw;
	sph_groestl512_context   ctx_groestl;
	sph_jh512_context        ctx_jh;
	sph_keccak512_context    ctx_keccak;
	sph_skein512_context     ctx_skein;
	sm3_ctx_t                ctx_sm3;
	sph_cubehash512_context  ctx_cubehash;
	sph_shavite512_context   ctx_shavite;
	sph_simd512_context      ctx_simd;
	sph_echo512_context      ctx_echo;
	sph_hamsi512_context     ctx_hamsi;
	sph_fugue512_context     ctx_fugue;

	sph_blake512_init(&ctx_blake);
	sph_blake512(&ctx_blake, input, 80);
	sph_blake512_close(&ctx_blake, hash);

	sph_bmw512_init(&ctx_bmw);
	sph_bmw512(&ctx_bmw, hash, 64);
	sph_bmw512_close(&ctx_bmw, hash + 64);

	sph_groestl512_init(&ctx_groestl);
	sph_groestl512(&ctx_groestl, hash + 64, 64);
	sph_groestl512_close(&ctx_groestl, hash);

	sph_skein512_init(&ctx_skein);
	sph_skein512(&ctx_skein, hash, 64);
	sph_skein512_close(&ctx_skein, hash + 64);

	sph_jh512_init(&ctx_jh);
	sph_jh512(&ctx_jh, hash + 64, 64);
	sph_jh512_close(&ctx_jh, hash);

	sph_keccak512_init(&ctx_keccak);
	sph_keccak512(&ctx_keccak, hash, 64);
	sph_keccak512_close(&ctx_keccak, hash + 64);

	memset(hash, 0, 64);	//sm3 is 256bit hash
	sm3_init(&ctx_sm3);
	sph_sm3(&ctx_sm3, hash + 64, 64);
	sph_sm3_close(&ctx_sm3, hash);

	sph_cubehash512_init(&ctx_cubehash);
	sph_cubehash512(&ctx_cubehash, hash, 64);
	sph_cubehash512_close(&ctx_cubehash, hash + 64);

	sph_shavite512_init(&ctx_shavite);
	sph_shavite512(&ctx_shavite, hash + 64, 64);
	sph_shavite512_close(&ctx_shavite, hash);

	sph_simd512_init(&ctx_simd);
	sph_simd512(&ctx_simd, hash, 64);
	sph_simd512_close(&ctx_simd, hash + 64);

	sph_echo512_init(&ctx_echo);
	sph_echo512(&ctx_echo, hash + 64, 64);
	sph_echo512_close(&ctx_echo, hash);

	sph_hamsi512_init(&ctx_hamsi);
	sph_hamsi512(&ctx_hamsi, hash, 64);
	sph_hamsi512_close(&ctx_hamsi, hash + 64);

	sph_fugue512_init(&ctx_fugue);
	sph_fugue512(&ctx_fugue, hash + 64, 64);
	sph_fugue512_close(&ctx_fugue, hash);

	memcpy(output, hash, 32);
}

int scanhash_x13(int thr_id, struct work *work, uint32_t max_nonce, uint64_t *hashes_done)
{
	uint32_t _ALIGN(32) hash64[8];
	uint32_t endiandata[32];
	uint32_t *pdata = work->data;
	uint32_t *ptarget = work->target;

	const uint32_t first_nonce = pdata[19];
	const uint32_t Htarg = ptarget[7];

	uint32_t n = pdata[19] - 1;

	uint64_t htmax[] = {
		0,
		0xF,
		0xFF,
		0xFFF,
		0xFFFF,
		0x10000000
	};
	uint32_t masks[] = {
		0xFFFFFFFF,
		0xFFFFFFF0,
		0xFFFFFF00,
		0xFFFFF000,
		0xFFFF0000,
		0
	};

	// we need bigendian data...
	int kk = 0;
	for (; kk < 32; kk++)
	{
		be32enc(&endiandata[kk], ((uint32_t*)pdata)[kk]);
	};

	do {
		pdata[19] = ++n;
		be32enc(&endiandata[19], n);
		x13hash(hash64, &endiandata);
		if (((hash64[7] & 0xFFFFFF00) == 0) &&
			fulltest(hash64, ptarget)) {
			*hashes_done = n - first_nonce + 1;
			return true;
		}
	} while (n < max_nonce && !work_restart[thr_id].restart);

	*hashes_done = n - first_nonce + 1;
	pdata[19] = n;
	return 0;
}
