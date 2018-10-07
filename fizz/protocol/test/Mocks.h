/*
 *  Copyright (c) 2018-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <fizz/crypto/aead/test/Mocks.h>
#include <fizz/crypto/exchange/test/Mocks.h>
#include <fizz/crypto/test/Mocks.h>
#include <fizz/protocol/Certificate.h>
#include <fizz/protocol/CertificateCompressor.h>
#include <fizz/protocol/CertificateVerifier.h>
#include <fizz/protocol/Factory.h>
#include <fizz/protocol/HandshakeContext.h>
#include <fizz/protocol/KeyScheduler.h>
#include <fizz/protocol/Types.h>
#include <fizz/record/test/Mocks.h>

/* using override */
using namespace testing;

namespace fizz {
namespace test {

class MockKeyScheduler : public KeyScheduler {
 public:
  MockKeyScheduler() : KeyScheduler(std::make_unique<MockKeyDerivation>()) {}

  MOCK_METHOD1(deriveEarlySecret, void(folly::ByteRange psk));
  MOCK_METHOD0(deriveHandshakeSecret, void());
  MOCK_METHOD1(deriveHandshakeSecret, void(folly::ByteRange ecdhe));
  MOCK_METHOD0(deriveMasterSecret, void());
  MOCK_METHOD1(deriveAppTrafficSecrets, void(folly::ByteRange transcript));
  MOCK_METHOD0(clearMasterSecret, void());
  MOCK_METHOD0(clientKeyUpdate, uint32_t());
  MOCK_METHOD0(serverKeyUpdate, uint32_t());
  MOCK_CONST_METHOD2(
      getSecret,
      std::vector<uint8_t>(EarlySecrets s, folly::ByteRange transcript));
  MOCK_CONST_METHOD2(
      getSecret,
      std::vector<uint8_t>(HandshakeSecrets s, folly::ByteRange transcript));
  MOCK_CONST_METHOD2(
      getSecret,
      std::vector<uint8_t>(MasterSecrets s, folly::ByteRange transcript));
  MOCK_CONST_METHOD1(getSecret, std::vector<uint8_t>(AppTrafficSecrets s));
  MOCK_CONST_METHOD3(
      getTrafficKey,
      TrafficKey(
          folly::ByteRange trafficSecret,
          size_t keyLength,
          size_t ivLength));
  MOCK_CONST_METHOD2(
      getResumptionSecret,
      Buf(folly::ByteRange, folly::ByteRange));

  void setDefaults() {
    ON_CALL(*this, getTrafficKey(_, _, _))
        .WillByDefault(InvokeWithoutArgs([]() {
          return TrafficKey{folly::IOBuf::copyBuffer("key"),
                            folly::IOBuf::copyBuffer("iv")};
        }));
    ON_CALL(*this, getResumptionSecret(_, _))
        .WillByDefault(InvokeWithoutArgs(
            []() { return folly::IOBuf::copyBuffer("resumesecret"); }));
  }
};

class MockHandshakeContext : public HandshakeContext {
 public:
  MOCK_METHOD1(appendToTranscript, void(const Buf& transcript));
  MOCK_CONST_METHOD0(getHandshakeContext, Buf());
  MOCK_CONST_METHOD1(getFinishedData, Buf(folly::ByteRange baseKey));
  MOCK_CONST_METHOD0(getBlankContext, folly::ByteRange());

  void setDefaults() {
    ON_CALL(*this, getHandshakeContext()).WillByDefault(InvokeWithoutArgs([]() {
      return folly::IOBuf::copyBuffer("context");
    }));

    ON_CALL(*this, getFinishedData(_)).WillByDefault(InvokeWithoutArgs([]() {
      return folly::IOBuf::copyBuffer("verifydata");
    }));
  }
};

class MockCert : public Cert {
 public:
  MOCK_CONST_METHOD0(getIdentity, std::string());
  MOCK_CONST_METHOD0(getX509, folly::ssl::X509UniquePtr());
};

class MockSelfCert : public SelfCert {
 public:
  MOCK_CONST_METHOD0(getIdentity, std::string());
  MOCK_CONST_METHOD0(getAltIdentities, std::vector<std::string>());
  MOCK_CONST_METHOD0(getSigSchemes, std::vector<SignatureScheme>());

  MOCK_CONST_METHOD1(_getCertMessage, CertificateMsg(Buf&));
  CertificateMsg getCertMessage(Buf buf) const override {
    return _getCertMessage(buf);
  }
  MOCK_CONST_METHOD1(
      getCompressedCert,
      CompressedCertificate(CertificateCompressionAlgorithm));

  MOCK_CONST_METHOD3(
      sign,
      Buf(SignatureScheme scheme,
          CertificateVerifyContext context,
          folly::ByteRange toBeSigned));
  MOCK_CONST_METHOD0(getX509, folly::ssl::X509UniquePtr());
};

class MockPeerCert : public PeerCert {
 public:
  MOCK_CONST_METHOD0(getIdentity, std::string());
  MOCK_CONST_METHOD4(
      verify,
      void(
          SignatureScheme scheme,
          CertificateVerifyContext context,
          folly::ByteRange toBeSigned,
          folly::ByteRange signature));
  MOCK_CONST_METHOD0(getX509, folly::ssl::X509UniquePtr());
};

class MockCertificateVerifier : public CertificateVerifier {
 public:
  MOCK_CONST_METHOD1(
      verify,
      void(const std::vector<std::shared_ptr<const PeerCert>>&));

  MOCK_CONST_METHOD0(getCertificateRequestExtensions, std::vector<Extension>());
};

class MockFactory : public Factory {
 public:
  MOCK_CONST_METHOD0(
      makePlaintextReadRecordLayer,
      std::unique_ptr<PlaintextReadRecordLayer>());
  MOCK_CONST_METHOD0(
      makePlaintextWriteRecordLayer,
      std::unique_ptr<PlaintextWriteRecordLayer>());
  MOCK_CONST_METHOD1(
      makeEncryptedReadRecordLayer,
      std::unique_ptr<EncryptedReadRecordLayer>(
          EncryptionLevel encryptionLevel));
  MOCK_CONST_METHOD1(
      makeEncryptedWriteRecordLayer,
      std::unique_ptr<EncryptedWriteRecordLayer>(
          EncryptionLevel encryptionLevel));
  MOCK_CONST_METHOD1(
      makeKeyScheduler,
      std::unique_ptr<KeyScheduler>(CipherSuite cipher));
  MOCK_CONST_METHOD1(
      makeHandshakeContext,
      std::unique_ptr<HandshakeContext>(CipherSuite cipher));
  MOCK_CONST_METHOD1(
      makeKeyExchange,
      std::unique_ptr<KeyExchange>(NamedGroup group));
  MOCK_CONST_METHOD1(makeAead, std::unique_ptr<Aead>(CipherSuite cipher));
  MOCK_CONST_METHOD0(makeRandom, Random());
  MOCK_CONST_METHOD0(makeTicketAgeAdd, uint32_t());

  MOCK_CONST_METHOD1(_makePeerCert, std::shared_ptr<PeerCert>(Buf&));
  std::shared_ptr<PeerCert> makePeerCert(Buf certData) const override {
    return _makePeerCert(certData);
  }

  void setDefaults() {
    ON_CALL(*this, makePlaintextReadRecordLayer())
        .WillByDefault(InvokeWithoutArgs(
            []() { return std::make_unique<MockPlaintextReadRecordLayer>(); }));

    ON_CALL(*this, makePlaintextWriteRecordLayer())
        .WillByDefault(InvokeWithoutArgs([]() {
          auto ret = std::make_unique<MockPlaintextWriteRecordLayer>();
          ret->setDefaults();
          return ret;
        }));
    ON_CALL(*this, makeEncryptedReadRecordLayer(_))
        .WillByDefault(Invoke([](EncryptionLevel encryptionLevel) {
          return std::make_unique<MockEncryptedReadRecordLayer>(
              encryptionLevel);
        }));

    ON_CALL(*this, makeEncryptedWriteRecordLayer(_))
        .WillByDefault(Invoke([](EncryptionLevel encryptionLevel) {
          auto ret =
              std::make_unique<MockEncryptedWriteRecordLayer>(encryptionLevel);
          ret->setDefaults();
          return ret;
        }));

    ON_CALL(*this, makeKeyScheduler(_)).WillByDefault(InvokeWithoutArgs([]() {
      auto ret = std::make_unique<MockKeyScheduler>();
      ret->setDefaults();
      return ret;
    }));
    ON_CALL(*this, makeHandshakeContext(_))
        .WillByDefault(InvokeWithoutArgs([]() {
          auto ret = std::make_unique<MockHandshakeContext>();
          ret->setDefaults();
          return ret;
        }));
    ON_CALL(*this, makeKeyExchange(_)).WillByDefault(InvokeWithoutArgs([]() {
      auto ret = std::make_unique<MockKeyExchange>();
      ret->setDefaults();
      return ret;
    }));
    ON_CALL(*this, makeAead(_)).WillByDefault(InvokeWithoutArgs([]() {
      auto ret = std::make_unique<MockAead>();
      ret->setDefaults();
      return ret;
    }));
    ON_CALL(*this, makeRandom()).WillByDefault(InvokeWithoutArgs([]() {
      Random random;
      random.fill(0x44);
      return random;
    }));
    ON_CALL(*this, makeTicketAgeAdd()).WillByDefault(InvokeWithoutArgs([]() {
      return 0x44444444;
    }));
    ON_CALL(*this, _makePeerCert(_)).WillByDefault(InvokeWithoutArgs([]() {
      return std::make_unique<MockPeerCert>();
    }));
  }
};

class MockCertificateDecompressor : public CertificateDecompressor {
 public:
  MOCK_CONST_METHOD0(getAlgorithm, CertificateCompressionAlgorithm());
  MOCK_METHOD1(decompress, CertificateMsg(const CompressedCertificate&));
  void setDefaults() {
    ON_CALL(*this, getAlgorithm()).WillByDefault(InvokeWithoutArgs([]() {
      return CertificateCompressionAlgorithm::zlib;
    }));
  }
};

class MockCertificateCompressor : public CertificateCompressor {
 public:
  MOCK_CONST_METHOD0(getAlgorithm, CertificateCompressionAlgorithm());
  MOCK_METHOD1(compress, CompressedCertificate(const CertificateMsg&));
  void setDefaults() {
    ON_CALL(*this, getAlgorithm()).WillByDefault(InvokeWithoutArgs([]() {
      return CertificateCompressionAlgorithm::zlib;
    }));
  }
};

} // namespace test
} // namespace fizz
