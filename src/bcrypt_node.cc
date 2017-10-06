#include <napi.h>

#include <string>
#include <cstring>
#include <vector>
#include <stdlib.h> // atoi

#include "node_blf.h"

#define NODE_LESS_THAN (!(NODE_VERSION_AT_LEAST(0, 5, 4)))

namespace {

bool ValidateSalt(const char* salt) {

    if (!salt || *salt != '$') {
        return false;
    }

    // discard $
    salt++;

    if (*salt > BCRYPT_VERSION) {
        return false;
    }

    if (salt[1] != '$') {
        switch (salt[1]) {
        case 'a':
            salt++;
            break;
        default:
            return false;
        }
    }

    // discard version + $
    salt += 2;

    if (salt[2] != '$') {
        return false;
    }

    int n = atoi(salt);
    if (n > 31 || n < 0) {
        return false;
    }

    if (((uint8_t)1 << (uint8_t)n) < BCRYPT_MINROUNDS) {
        return false;
    }

    salt += 3;
    if (strlen(salt) * 3 / 4 < BCRYPT_MAXSALT) {
        return false;
    }

    return true;
}

/* SALT GENERATION */

/*class SaltAsyncWorker : public Nan::AsyncWorker {
public:
    SaltAsyncWorker(Nan::Callback *callback, std::string seed, ssize_t rounds)
        : Nan::AsyncWorker(callback), seed(seed), rounds(rounds) {
    }

    ~SaltAsyncWorker() {}

    void Execute() {
        char salt[_SALT_LEN];
        bcrypt_gensalt(rounds, (u_int8_t *)&seed[0], salt);
        this->salt = std::string(salt);
    }

    void HandleOKCallback() {
        Nan::HandleScope scope;

        Local<Value> argv[2];
        argv[0] = Nan::Undefined();
        argv[1] = Nan::Encode(salt.c_str(), salt.size(), Nan::BINARY);
        callback->Call(2, argv);
    }

private:
    std::string seed;
    std::string salt;
    ssize_t rounds;
};*/

/*NAN_METHOD(GenerateSalt) {
    Nan::HandleScope scope;

    if (info.Length() < 3) {
        Nan::ThrowTypeError("3 arguments expected");
        return;
    }

    if (!Buffer::HasInstance(info[1]) || Buffer::Length(info[1].As<Object>()) != 16) {
        Nan::ThrowTypeError("Second argument must be a 16 byte Buffer");
        return;
    }

    const int32_t rounds = Nan::To<int32_t>(info[0]).FromMaybe(0);
    Local<Object> seed = info[1].As<Object>();
    Local<Function> callback = Local<Function>::Cast(info[2]);

    SaltAsyncWorker* saltWorker = new SaltAsyncWorker(new Nan::Callback(callback),
        std::string(Buffer::Data(seed), 16), rounds);

    Nan::AsyncQueueWorker(saltWorker);
}*/

/*NAN_METHOD(GenerateSaltSync) {
    Nan::HandleScope scope;

    if (info.Length() < 2) {
        Nan::ThrowTypeError("2 arguments expected");
        return;
    }

    if (!Buffer::HasInstance(info[1]) || Buffer::Length(info[1].As<Object>()) != 16) {
        Nan::ThrowTypeError("Second argument must be a 16 byte Buffer");
        return;
    }

    const int32_t rounds = Nan::To<int32_t>(info[0]).FromMaybe(0);
    u_int8_t* seed = (u_int8_t*)Buffer::Data(info[1].As<Object>());

    char salt[_SALT_LEN];
    bcrypt_gensalt(rounds, seed, salt);

    info.GetReturnValue().Set(Nan::Encode(salt, strlen(salt), Nan::BINARY));
}*/

/* ENCRYPT DATA - USED TO BE HASHPW */

/*class EncryptAsyncWorker : public Nan::AsyncWorker {
  public:
    EncryptAsyncWorker(Nan::Callback *callback, std::string input, std::string salt)
        : Nan::AsyncWorker(callback), input(input), salt(salt) {
    }

    ~EncryptAsyncWorker() {}

    void Execute() {
        if (!(ValidateSalt(salt.c_str()))) {
            error = "Invalid salt. Salt must be in the form of: $Vers$log2(NumRounds)$saltvalue";
        }

        char bcrypted[_PASSWORD_LEN];
        bcrypt(input.c_str(), salt.c_str(), bcrypted);
        output = std::string(bcrypted);
    }

    void HandleOKCallback() {
        Nan::HandleScope scope;

        Local<Value> argv[2];

        if (!error.empty()) {
            argv[0] = Nan::Error(error.c_str());
            argv[1] = Nan::Undefined();
        } else {
            argv[0] = Nan::Undefined();
            argv[1] = Nan::Encode(output.c_str(), output.size(), Nan::BINARY);
        }

        callback->Call(2, argv);
    }

  private:
    std::string input;
    std::string salt;
    std::string error;
    std::string output;
};*/

/*NAN_METHOD(Encrypt) {
    Nan::HandleScope scope;

    if (info.Length() < 3) {
        Nan::ThrowTypeError("3 arguments expected");
        return;
    }

    Nan::Utf8String data(info[0]->ToString());
    Nan::Utf8String salt(info[1]->ToString());
    Local<Function> callback = Local<Function>::Cast(info[2]);

    EncryptAsyncWorker* encryptWorker = new EncryptAsyncWorker(new Nan::Callback(callback),
        std::string(*data), std::string(*salt));

    Nan::AsyncQueueWorker(encryptWorker);
}*/

/*NAN_METHOD(EncryptSync) {
    Nan::HandleScope scope;

    if (info.Length() < 2) {
        Nan::ThrowTypeError("2 arguments expected");
        info.GetReturnValue().Set(Nan::Undefined());
        return;
    }

    Nan::Utf8String data(info[0]->ToString());
    Nan::Utf8String salt(info[1]->ToString());

    if (!(ValidateSalt(*salt))) {
        Nan::ThrowError("Invalid salt. Salt must be in the form of: $Vers$log2(NumRounds)$saltvalue");
        info.GetReturnValue().Set(Nan::Undefined());
        return;
    }

    char bcrypted[_PASSWORD_LEN];
    bcrypt(*data, *salt, bcrypted);
    info.GetReturnValue().Set(Nan::Encode(bcrypted, strlen(bcrypted), Nan::BINARY));
}*/

Napi::Value EncryptSync(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    if (info.Length() < 2) {
        throw Napi::TypeError::New(info.Env(), "2 arguments expected");
        return info.Env().Undefined();     
    }
    std::string data = info[0].As<Napi::String>();;
    std::string salt = info[1].As<Napi::String>();;
    if (!(ValidateSalt(salt.c_str()))) {
        throw Napi::Error::New(info.Env(), "Invalid salt. Salt must be in the form of: $Vers$log2(NumRounds)$saltvalue");
        return info.Env().Undefined();
    }
    char bcrypted[_PASSWORD_LEN];
    bcrypt(data.c_str(), salt.c_str(), bcrypted);
    return Napi::String::New(env, bcrypted, strlen(bcrypted));
}


/* COMPARATOR */

bool CompareStrings(const char* s1, const char* s2) {

    bool eq = true;
    int s1_len = strlen(s1);
    int s2_len = strlen(s2);

    if (s1_len != s2_len) {
        eq = false;
    }

    const int max_len = (s2_len < s1_len) ? s1_len : s2_len;

    // to prevent timing attacks, should check entire string
    // don't exit after found to be false
    for (int i = 0; i < max_len; ++i) {
      if (s1_len >= i && s2_len >= i && s1[i] != s2[i]) {
        eq = false;
      }
    }

    return eq;
}

/*class CompareAsyncWorker : public Nan::AsyncWorker {
  public:
    CompareAsyncWorker(Nan::Callback *callback, std::string input, std::string encrypted)
        : Nan::AsyncWorker(callback), input(input), encrypted(encrypted) {

        result = false;
    }

    ~CompareAsyncWorker() {}

    void Execute() {
        char bcrypted[_PASSWORD_LEN];
        if (ValidateSalt(encrypted.c_str())) {
            bcrypt(input.c_str(), encrypted.c_str(), bcrypted);
            result = CompareStrings(bcrypted, encrypted.c_str());
        }
    }

    void HandleOKCallback() {
        Nan::HandleScope scope;

        Local<Value> argv[2];
        argv[0] = Nan::Undefined();
        argv[1] = Nan::New<Boolean>(result);
        callback->Call(2, argv);
    }

  private:
    std::string input;
    std::string encrypted;
    bool result;
};*/

/*NAN_METHOD(Compare) {
    Nan::HandleScope scope;

    if (info.Length() < 3) {
        Nan::ThrowTypeError("3 arguments expected");
        return;
    }

    Nan::Utf8String input(info[0]->ToString());
    Nan::Utf8String encrypted(info[1]->ToString());
    Local<Function> callback = Local<Function>::Cast(info[2]);

    CompareAsyncWorker* compareWorker = new CompareAsyncWorker(new Nan::Callback(callback),
        std::string(*input), std::string(*encrypted));

    Nan::AsyncQueueWorker(compareWorker);
}*/

Napi::Value CompareSync(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    if (info.Length() < 2) {
        throw Napi::TypeError::New(info.Env(), "2 arguments expected");
        return info.Env().Undefined();     
    }
    std::string pw = info[0].As<Napi::String>();
    std::string hash = info[1].As<Napi::String>();
    char bcrypted[_PASSWORD_LEN];
    if (ValidateSalt(hash.c_str())) {
        bcrypt(pw.c_str(), hash.c_str(), bcrypted);
        return Napi::Boolean::New(env, CompareStrings(bcrypted, hash.c_str()));
    } else {
        return Napi::Boolean::New(env, false);
    }
}

Napi::Value GetRounds(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    if (info.Length() < 1) {
        throw Napi::TypeError::New(info.Env(), "1 argument expected");
        return info.Env().Undefined();     
    }
    Napi::String hashed = info[0].As<Napi::String>();
    std::string hash = hashed.ToString();
    const char* bcrypt_hash = hash.c_str();
    u_int32_t rounds;
    if (!(rounds = bcrypt_get_rounds(bcrypt_hash))) {
        throw Napi::Error::New(info.Env(), "invalid hash provided");
        return info.Env().Undefined();
    }
    return Napi::Number::New(env, rounds);
}

} // anonymous namespace

Napi::Object init(Napi::Env env, Napi::Object exports) {
    //exports.Set(Napi::String::New(env, "gen_salt_sync"), Napi::Function::New(env, GenerateSaltSync));
    exports.Set(Napi::String::New(env, "encrypt_sync"), Napi::Function::New(env, EncryptSync));
    exports.Set(Napi::String::New(env, "compare_sync"), Napi::Function::New(env, CompareSync));
    exports.Set(Napi::String::New(env, "get_rounds"), Napi::Function::New(env, GetRounds));
    //exports.Set(Napi::String::New(env, "gen_salt"), Napi::Function::New(env, GenerateSalt));
    //exports.Set(Napi::String::New(env, "encrypt"), Napi::Function::New(env, Encrypt));
    //exports.Set(Napi::String::New(env, "compare"), Napi::Function::New(env, Compare));
    return exports;
};

NODE_API_MODULE(bcrypt_lib, init);
