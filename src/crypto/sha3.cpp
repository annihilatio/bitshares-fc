

#include <fc/crypto/sha3.hpp>
#include <fc/exception/exception.hpp>
//
// Created by user on 19.10.18.
//
namespace dev
{
    namespace openssl {

        sha3_224_encoder::sha3_224_encoder()
                : sha3_encoder_base(EVP_sha3_224) {
            EVP_DigestInit_ex(ctx, EVP_sha3_224(), NULL);
        }

        sha3_224_encoder::~sha3_224_encoder() {}

        sha3_256_encoder::sha3_256_encoder()
                : sha3_encoder_base(EVP_sha3_256) {
            EVP_DigestInit_ex(ctx, EVP_sha3_256(), NULL);
        }

        sha3_256_encoder::~sha3_256_encoder() {}

        sha3_512_encoder::sha3_512_encoder()
                : sha3_encoder_base(EVP_sha3_512) {
            EVP_DigestInit_ex(ctx, EVP_sha3_512(), NULL);
        }

        sha3_512_encoder::~sha3_512_encoder() {}

        bool sha3(bytesConstRef _input, bytesRef o_output) noexcept {
            if (o_output.size() != dev::h256::size)
                return false;
            sha3_256_encoder enc;
            enc.write((char *) _input.data(), _input.size());
            enc.result((char *) o_output.data(), o_output.size());
            return true;
        }

        bool sha3_512(bytesConstRef _input, bytesRef o_output) noexcept {
            if (o_output.size() != dev::h512::size)
                return false;
            sha3_512_encoder enc;
            enc.write((char *) _input.data(), _input.size());
            enc.result((char *) o_output.data(), o_output.size());
            return true;
        }
    }
}
