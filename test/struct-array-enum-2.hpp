struct ztd
{
	typedef unsigned char u8;
	enum { sha224_hash_size = 28 };

	struct decl_hash;
	typedef struct decl_hash decl_hash;

	struct decl_hash {
		u8 sum[sha224_hash_size];
	};
};
