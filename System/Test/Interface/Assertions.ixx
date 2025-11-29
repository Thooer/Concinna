module;

export module Sys.Job.Test:Assertions;

import <cmath>;
import <source_location>;
import <type_traits>;

import Lang;

import :Core;
import :Context;
import :Registration;

export namespace Sys {

    namespace Detail {
        inline void ReportFailure(
            ITestContext& ctx,
            const char* expression,
            const char* note,
            std::source_location loc) noexcept {
            const char* message = note ? note : expression;
            if (message) {
                ctx.Logger().Error(message);
            }
            TestResultBuilder::Failure(message, loc.file_name(), static_cast<Int32>(loc.line())).BuildAndReport(ctx);
        }
    }

    template<typename T>
    class Expectation {
    public:
        Expectation(ITestContext& ctx, T value, bool hard, std::source_location loc) noexcept
            : m_ctx(&ctx), m_value(value), m_hard(hard), m_location(loc) {}

        template<typename U>
        Expectation& ToBe(const U& expected, const char* note = nullptr) noexcept {
            if (!(m_value == expected)) {
                Detail::ReportFailure(*m_ctx, "Expectation.ToBe", note, m_location);
                if (m_hard) { FailFast(); }
            }
            return *this;
        }

        template<typename U>
        Expectation& NotToBe(const U& unexpected, const char* note = nullptr) noexcept {
            if (m_value == unexpected) {
                Detail::ReportFailure(*m_ctx, "Expectation.NotToBe", note, m_location);
                if (m_hard) { FailFast(); }
            }
            return *this;
        }

        Expectation& ToBeApproximately(T expected, T tolerance, const char* note = nullptr) noexcept {
            T diff = static_cast<T>(std::fabs(static_cast<double>(m_value - expected)));
            if (diff > tolerance) {
                Detail::ReportFailure(*m_ctx, "Expectation.ToBeApproximately", note, m_location);
                if (m_hard) { FailFast(); }
            }
            return *this;
        }

        template<typename U>
        Expectation& ToBeGreaterThan(const U& expected, const char* note = nullptr) noexcept {
            if (!(m_value > expected)) {
                Detail::ReportFailure(*m_ctx, "Expectation.ToBeGreaterThan", note, m_location);
                if (m_hard) { FailFast(); }
            }
            return *this;
        }

        template<typename U>
        Expectation& ToBeLessThan(const U& expected, const char* note = nullptr) noexcept {
            if (!(m_value < expected)) {
                Detail::ReportFailure(*m_ctx, "Expectation.ToBeLessThan", note, m_location);
                if (m_hard) { FailFast(); }
            }
            return *this;
        }

    private:
        ITestContext* m_ctx;
        T m_value;
        bool m_hard;
        std::source_location m_location;
    };

    template<typename P>
    class Expectation<P*> {
    public:
        Expectation(ITestContext& ctx, P* value, bool hard, std::source_location loc) noexcept
            : m_ctx(&ctx), m_value(value), m_hard(hard), m_location(loc) {}

        Expectation& IsNull(const char* note = nullptr) noexcept {
            if (m_value != nullptr) {
                Detail::ReportFailure(*m_ctx, "Expectation.IsNull", note, m_location);
                if (m_hard) { FailFast(); }
            }
            return *this;
        }

        Expectation& IsNotNull(const char* note = nullptr) noexcept {
            if (m_value == nullptr) {
                Detail::ReportFailure(*m_ctx, "Expectation.IsNotNull", note, m_location);
                if (m_hard) { FailFast(); }
            }
            return *this;
        }

    private:
        ITestContext* m_ctx;
        P* m_value;
        bool m_hard;
        std::source_location m_location;
    };

    template<typename T>
    [[nodiscard]] inline Expectation<std::decay_t<T>> That(
        T&& actual,
        ITestContext& ctx,
        bool hard = false,
        std::source_location loc = std::source_location::current()) noexcept {
        return Expectation<std::decay_t<T>>(ctx, Forward<T>(actual), hard, loc);
    }

    template<typename T>
    [[nodiscard]] inline Expectation<std::decay_t<T>> ExpectValue(
        T&& actual,
        ITestContext& ctx,
        bool hard = false,
        std::source_location loc = std::source_location::current()) noexcept {
        return Expectation<std::decay_t<T>>(ctx, Forward<T>(actual), hard, loc);
    }
}
