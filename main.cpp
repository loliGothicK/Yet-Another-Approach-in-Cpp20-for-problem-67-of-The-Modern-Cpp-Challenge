/**
 * @file main.cpp
 *
 * @brief This is yet another approach in C++20 for problem-67 of "The Modern C++ Challenge"
 *
 *        Design:
 *          - class password_validator
 *              It is implemented by a strategy pattern.
 *              This class holds the validation rules and is responsible for their execution and the aggregation of the results.
 *              The rules (how passwords are validated) are injected in the member function and the class itself is not concerned.
 *
 *          - validation results
 *              The feature/cpp20 branch of Mitama.Result is used.
 *              This library is a ported version of Rust's Result type to C++.
 *              The type `result<T, E>` is a kind of variant.
 *              By using it, you can get information about the success or failure of the validation.
 *              Even if it fails, you can get error messages.
 *
 *              There is a serious UX problem with the original answer.
 *              In the original answer, the validation only returns a bool value.
 *
 *              Consider the case of using validation as an actual API.
 *              The validation is triggered by the user entering a password.
 *              And the result of the validation should be fed back to the UI.
 *              In the original implementation, the limit would be whether the password input field would be green or red.
 *              But that wouldn't let the user know what went wrong.
 *              This implementation improves on this point.
 *              The result type collects and returns error messages
 *              so that the user not only knows that there was a problem
 *              with the password they entered, but also knows what to fix.
 *
 */
#include <mitama/result/result.hpp>
#include <iostream>
#include <vector>
#include <functional>
#include <ranges>
#include <concepts>

namespace validate {
  // alias template for result<(), E>
  template<class E>
  using result = mitama::result<std::monostate, E>;

  class password_validator {
    using validate_result = mitama::result<std::monostate, std::string>;
    std::vector<std::function<validate_result(std::string_view)>> validators;
  public:
    password_validator() = default;

    /// Add a new validate rule.
    /// \param rule predicate functor for a new rule (requires std::predicate<std::string_view>).
    /// \param msg error message for a new rule.
    /// \return *this
    auto &rule(std::predicate<std::string_view> auto &&rule, std::string msg) {
      validators.emplace_back(
              [msg, rule = std::forward<decltype(rule)>(rule)](std::string_view password) -> validate_result {
                if (rule(password)) {
                  return mitama::success();
                } else {
                  return mitama::failure(msg);
                }
              });
      return *this;
    }

    /// executes validation against `password`.
    /// \param password target for validate
    /// \return returns `success(())` if there is no validate errors, otherwise return `failure([error messages])`
    auto validate(std::string_view password) & -> result<std::vector<std::string>> {
      std::vector<validate_result> results;
      std::transform(validators.begin(), validators.end(), std::back_inserter(results),
                     [&](auto &validator) { return validator(password); });
      if (std::all_of(results.begin(), results.end(), [](auto &res) { return res.is_ok(); })) {
        return mitama::success();
      } else {
        auto errors = results | std::ranges::views::filter(&validate_result::is_err);
        std::vector<std::string> error_messages;
        std::transform(errors.begin(), errors.end(), std::back_inserter(error_messages),
                       [](auto error) { return error.unwrap_err(); });

        return mitama::failure(error_messages);
      }
    }
  };
}

// example
#include <mitama/result/result_io.hpp> // for pretty outputs
#include <cctype>
int main() {
  using namespace std::literals;

  validate::password_validator validator;

  auto res = validator
          .rule( // for password length
                  [](std::string_view password) {
                    return password.length() > 8;
                  },
                  "password length must be greater than 8 chars."
          )
          .rule( // for digit
                  [](std::string_view password) {
                    return password.find_first_of("0123456789") != std::string::npos;
                  },
                  "password must contain a digit."
          )
          .rule( // for case
                  [](std::string_view password) {
                    auto pred = [](int(&fn)(int)) {
                      return [&](char ch) -> bool { return fn(static_cast<unsigned char>(ch)); };
                    };

                    bool has_lower = std::any_of(password.begin(), password.end(), pred(std::islower));
                    bool has_upper = std::any_of(password.begin(), password.end(), pred(std::isupper));
                    return has_lower && has_upper;
                  },
                  "password must contain both of lower and upper case."
          )
          .validate("hogehogeho"sv);

  std::cout << res << std::endl;
  return 0;
}
