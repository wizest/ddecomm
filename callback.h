#ifndef CALLBACK_H
#define CALLBACK_H

// get a WinAPI callback from a lambda by Andy Prowl
// http://stackoverflow.com/questions/14343428/pass-as-callback-the-address-of-a-static-method-whose-signature-came-by-tuple-un

#include <type_traits>
#include <memory>

template<typename F>
struct singleton
{
    static std::unique_ptr<F> instance;
    static void set_instance(F f) { instance.reset(new F(f)); }
};

template<typename F>
std::unique_ptr<F> singleton<F>::instance;

template<typename F, typename... Ts>
typename std::result_of<F(Ts...)>::type __stdcall lambda_caller(Ts... args)
{
    if (singleton<F>::instance == nullptr)
    {
        // throw some exception...
        throw std::invalid_argument("nullptr");
    }
    else
    {
        return (*(singleton<F>::instance))(args...);
    }
}

#define get_api_callback(lambda) \
    &lambda_caller<decltype(lambda)>; singleton<decltype(lambda)>::set_instance(lambda);

// How to use 1
//    #include <iostream>
//    int main()
//    {
//        //Example lambda
//        int toBeCaptured = 8;
//        auto lambda =
//            [&](std::string& str) -> size_t{
//                return toBeCaptured + str.length();
//            };
//        singleton<decltype(lambda)>::set_instance(lambda);
//        size_t (__stdcall *pfn)(std::string&) = &lambda_caller<decltype(lambda)>;
//        std::string str = "hello";
//        int out = pfn(str);
//        std::cout << out;
//        return 0;
//    }

// How to use 2
//    #include <iostream>
//    int main()
//    {
//        //Example lambda
//        int toBeCaptured = 8;
//        auto lambda =
//            [&](std::string& str) -> size_t{
//                return toBeCaptured + str.length();
//            };
//        // As simple as that...
//        size_t (__stdcall *pfn)(std::string&) = get_api_callback(lambda);
//        std::string str = "hello";
//        int out = pfn(str);
//        std::cout << out;
//        return 0;
//    }

#endif // CALLBACK_H

