#include <cpr/cpr.h>
#include <iostream>
#include <ixwebsocket/IXWebSocket.h>
#include <openssl/hmac.h>
#include <openssl/evp.h>
#include <openssl/pem.h>
#include <openssl/bio.h>
#include <openssl/buffer.h>
#include <string>
#include <uuid.h>
#include <random>
#include <stdexcept>
#include "BinanceAuth.hpp"

std::array<std::string, 2> getBinanceKeys() {
	std::ifstream file(".env");
	std::array<std::string, 2> keys;
	std::string line;
	if (!std::getline(file, line)) throw std::runtime_error(".env missing BINANCE_API_KEY line");
	keys[0] = line.substr(std::string("BINANCE_API_KEY=").size());
	if (!std::getline(file, line)) throw std::runtime_error(".env missing BINANCE_SECRET_KEY line");
	keys[1] = line.substr(std::string("BINANCE_SECRET_KEY=").size());
	return keys;
}

std::array<std::string, 2> getBinanceEd25519Keys() {
	std::ifstream file(".env");
	std::array<std::string, 2> keys;
	std::string line;
	bool foundApiKey = false, foundKeyPath = false;
	while (std::getline(file, line)) {
		if (line.rfind("BINANCE_ED25519_API_KEY=", 0) == 0) {
			keys[0] = line.substr(std::string("BINANCE_ED25519_API_KEY=").size());
			foundApiKey = true;
		} else if (line.rfind("BINANCE_ED25519_PRIVATE_KEY_PATH=", 0) == 0) {
			keys[1] = line.substr(std::string("BINANCE_ED25519_PRIVATE_KEY_PATH=").size());
			foundKeyPath = true;
		}
	}
	if (!foundApiKey) throw std::runtime_error(".env missing BINANCE_ED25519_API_KEY line");
	if (!foundKeyPath) throw std::runtime_error(".env missing BINANCE_ED25519_PRIVATE_KEY_PATH line");
	return keys;
}

std::string hmacSha256Hex(const std::string& key, const std::string& data) {
	unsigned char* digest = HMAC(
		EVP_sha256(),
		key.c_str(), key.size(),
		reinterpret_cast<const unsigned char*>(data.c_str()), data.size(),
		nullptr, nullptr
	);

	std::ostringstream oss;
	for (int i = 0; i < 32; ++i) { // SHA256 = 32 bytes
		oss << std::hex << std::setw(2) << std::setfill('0') << (int)digest[i];
	}
	return oss.str();
}

int64_t getTimestampMillis() {
    return duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
}

std::string generateUserDataSignature(const std::string& queryString) {
	auto keys = getBinanceKeys();
	const std::string apiKey = keys[0];
	const std::string secretKey = keys[1];
	std::string signature = hmacSha256Hex(secretKey, queryString);
	return signature;
}

std::string signEd25519Base64(const std::string& privateKeyPath, const std::string& data) {
    FILE* fp = fopen(privateKeyPath.c_str(), "r");
    if (!fp) throw std::runtime_error("Could not open Ed25519 private key file: " + privateKeyPath);

    EVP_PKEY* pkey = PEM_read_PrivateKey(fp, nullptr, nullptr, nullptr);
    fclose(fp);
    if (!pkey) throw std::runtime_error("Failed to parse Ed25519 private key");

    EVP_MD_CTX* mdctx = EVP_MD_CTX_new();
    if (!mdctx) {
        EVP_PKEY_free(pkey);
        throw std::runtime_error("EVP_MD_CTX_new failed");
    }

    if (EVP_DigestSignInit(mdctx, nullptr, nullptr, nullptr, pkey) <= 0) {
        EVP_MD_CTX_free(mdctx);
        EVP_PKEY_free(pkey);
        throw std::runtime_error("EVP_DigestSignInit failed");
    }

    size_t sigLen = 0;
    if (EVP_DigestSign(mdctx, nullptr, &sigLen,
                        reinterpret_cast<const unsigned char*>(data.data()), data.size()) <= 0) {
        EVP_MD_CTX_free(mdctx);
        EVP_PKEY_free(pkey);
        throw std::runtime_error("EVP_DigestSign (size query) failed");
    }

    std::string sig(sigLen, '\0');
    if (EVP_DigestSign(mdctx, reinterpret_cast<unsigned char*>(sig.data()), &sigLen,
                        reinterpret_cast<const unsigned char*>(data.data()), data.size()) <= 0) {
        EVP_MD_CTX_free(mdctx);
        EVP_PKEY_free(pkey);
        throw std::runtime_error("EVP_DigestSign failed");
    }
    sig.resize(sigLen);

    EVP_MD_CTX_free(mdctx);
    EVP_PKEY_free(pkey);

    BIO* b64 = BIO_new(BIO_f_base64());
    BIO_set_flags(b64, BIO_FLAGS_BASE64_NO_NL);
    BIO* mem = BIO_new(BIO_s_mem());
    BIO_push(b64, mem);
    BIO_write(b64, sig.data(), static_cast<int>(sig.size()));
    BIO_flush(b64);

    BUF_MEM* bufferPtr;
    BIO_get_mem_ptr(b64, &bufferPtr);
    std::string encoded(bufferPtr->data, bufferPtr->length);
    BIO_free_all(b64);

    return encoded;
}

std::string generateRequestId() {
    static std::random_device rd;
    static auto seed_data = std::array<int, std::mt19937::state_size>{};
    static bool seeded = [] {
        std::generate(seed_data.begin(), seed_data.end(), std::ref(rd));
        return true;
    }();
    static std::seed_seq seq(seed_data.begin(), seed_data.end());
    static std::mt19937 generator(seq);
    static uuids::uuid_random_generator gen(generator);

    return uuids::to_string(gen());
}

std::string generateSessionLogonRequest() {
    auto keys = getBinanceEd25519Keys();
    const std::string apiKey = keys[0];
    const std::string privateKeyPath = keys[1];
    int64_t timestamp = getTimestampMillis();

    std::ostringstream payload;
    payload << "apiKey=" << apiKey << "&timestamp=" << timestamp;

    std::string signature = signEd25519Base64(privateKeyPath, payload.str());

    std::ostringstream request;
    request << "{"
            << "\"id\":\"" << generateRequestId() << "\","
            << "\"method\":\"session.logon\","
            << "\"params\":{"
                << "\"apiKey\":\"" << apiKey << "\","
                << "\"timestamp\":" << timestamp << ","
                << "\"signature\":\"" << signature << "\""
            << "}"
            << "}";
    return request.str();
}

std::string generateUserStreamRequest() {
	std::ostringstream request;
	request << "{"
	    << "\"id\":\"" << generateRequestId() << "\","
	    << "\"method\":\"userDataStream.subscribe\","
	    << "\"params\":{}"
	    << "}";
	return request.str();
}
