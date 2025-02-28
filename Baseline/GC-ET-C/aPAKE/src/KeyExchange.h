#include <emp-tool/emp-tool.h>

using namespace emp;
using namespace std;


vector<Integer> BIGNUM_to_vector_A(const BIGNUM* bn) {
    int num_bytes = BN_num_bytes(bn);  // Get byte size of BIGNUM
    vector<unsigned char> vec(num_bytes);

    BN_bn2bin(bn, vec.data());  // Convert BIGNUM to byte array

    vector<Integer> ret;

    for (unsigned char byte : vec) {
        ret.push_back(Integer(32, static_cast<int>(byte), ALICE));
    }

    return ret;
}

vector<Integer> BIGNUM_to_vector_B(const BIGNUM* bn) {
    int num_bytes = BN_num_bytes(bn);  // Get byte size of BIGNUM
    vector<unsigned char> vec(num_bytes);

    BN_bn2bin(bn, vec.data());  // Convert BIGNUM to byte array

    vector<Integer> ret;

    for (unsigned char byte : vec) {
        ret.push_back(Integer(32, static_cast<int>(byte), BOB));
    }
    
    return ret;
}

void generate_ecdh_key(EC_GROUP* group, BIGNUM*& private_key, EC_POINT*& public_key, BN_CTX* ctx) {
    private_key = BN_new();
    public_key = EC_POINT_new(group);

    BN_rand_range(private_key, EC_GROUP_get0_order(group));  // Generate private key
    EC_POINT_mul(group, public_key, private_key, NULL, NULL, ctx);  // Compute public key
}

// Function to compute shared secret
vector<Integer> compute_shared_secret_A(EC_GROUP* group, BIGNUM* private_key, const EC_POINT* peer_pub, BN_CTX* ctx) {
    EC_POINT* shared_point = EC_POINT_new(group);
    EC_POINT_mul(group, shared_point, NULL, peer_pub, private_key, ctx);

    BIGNUM* shared_secret_bn = BN_new();
    vector<Integer> secret_emp = BIGNUM_to_vector_A(shared_secret_bn);
    EC_POINT_get_affine_coordinates_GFp(group, shared_point, shared_secret_bn, NULL, ctx);

    vector<unsigned char> secret(BN_num_bytes(shared_secret_bn));
    BN_bn2bin(shared_secret_bn, secret.data());

    vector<Integer> ret;
    for (unsigned char byte : secret) {
        ret.push_back(Integer(32, static_cast<int>(byte), ALICE));
    }
    

    EC_POINT_free(shared_point);
    BN_free(shared_secret_bn);
    return ret;
}

vector<Integer> compute_shared_secret_B(EC_GROUP* group, BIGNUM* private_key, const EC_POINT* peer_pub, BN_CTX* ctx) {
    EC_POINT* shared_point = EC_POINT_new(group);
    EC_POINT_mul(group, shared_point, NULL, peer_pub, private_key, ctx);

    BIGNUM* shared_secret_bn = BN_new();
    vector<Integer> secret_emp = BIGNUM_to_vector_B(shared_secret_bn);
    EC_POINT_get_affine_coordinates_GFp(group, shared_point, shared_secret_bn, NULL, ctx);

    vector<unsigned char> secret(BN_num_bytes(shared_secret_bn));
    BN_bn2bin(shared_secret_bn, secret.data());

    vector<Integer> ret;
    for (unsigned char byte : secret) {
        ret.push_back(Integer(32, static_cast<int>(byte), BOB));
    }
    

    EC_POINT_free(shared_point);
    BN_free(shared_secret_bn);
    return ret;
}

vector<Integer> sha3_A(vector<Integer> input_vec, int length){
  vector<Integer> ret;
  uint8_t output_bytes[32];

  emp::Integer output = Integer(10, 32, ALICE);
  Integer* input = input_vec.data();

  SHA3_256_Calculator sha3_256_calculator = SHA3_256_Calculator();
  sha3_256_calculator.sha3_256(&output, input, length);

  output.reveal<uint8_t>(output_bytes, PUBLIC);

  for (int i=0; i<32; i++) {
    vector<Integer> to_insert = toBinary_(output_bytes[i]);
    ret.insert(ret.end(), to_insert.begin(), to_insert.end());
  }
  return ret;
}

vector<Integer> sha3_B(vector<Integer> input_vec, int length){
  vector<Integer> ret;
  uint8_t output_bytes[32];

  emp::Integer output = Integer(10, 32, BOB);
  Integer* input = input_vec.data();

  SHA3_256_Calculator sha3_256_calculator = SHA3_256_Calculator();
  sha3_256_calculator.sha3_256(&output, input, length);

  output.reveal<uint8_t>(output_bytes, PUBLIC);

  for (int i=0; i<32; i++) {
    vector<Integer> to_insert = toBinary_(output_bytes[i]);
    ret.insert(ret.end(), to_insert.begin(), to_insert.end());
  }
  return ret;
}


// Function to generate HMAC for authentication
vector<Integer> generate_hmac_A(vector<Integer> key, const std::string& message) {
    return sha3_A(key, key.size());
}

vector<Integer> generate_hmac_B(vector<Integer> key, const std::string& message) {
    return sha3_B(key, key.size());
}
