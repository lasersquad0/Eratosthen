#pragma once

#include "BigInt.h"
#include "ttmath/ttmath.h"

// can contain max ~192 decimal digits (=64*10*log10(2))
using TTMathBigInt = ttmath::UInt<10>;


struct BigIntPolicy
{
    using Type = BigInt;

    static void Sqrt(BigInt& obj)
    {
        obj = sqrt(obj);
    }

    static uint64_t ToUInt(const BigInt& obj)
    {
        return toUInt64(obj);
    }
};

struct TTMathBigIntPolicy
{
    using Type = TTMathBigInt;

    static void Sqrt(TTMathBigInt& obj)
    {
        obj.Sqrt();
    }

    static uint64_t ToUInt(const TTMathBigInt& obj)
    {
        return obj.ToUInt();
    }
};

template<typename Impl>
struct ProcessorTraits;

template<>
struct ProcessorTraits<BigInt>
{
    using Policy = BigIntPolicy;
};

template<>
struct ProcessorTraits<TTMathBigInt>
{
    using Policy = TTMathBigIntPolicy;
};

template<typename BigIntImpl>
class GenericBigInt : public BigIntImpl
{
private:
    using Policy = typename ProcessorTraits<BigIntImpl>::Policy;

public:
    using BigIntImpl::BigIntImpl; // use constructors from Impl type
    using BigIntImpl::operator=;
   
    //GenericBigInt(BigIntImpl& val): BigIntImpl(val) { };

    // function which signature differs in BigInt and TTMathBigInt
    void Sqrt()
    {
        Policy::Sqrt(*this);
    }

    // function which signature differs in BigInt and TTMathBigInt
    uint64_t ToUInt() const
    {
        return Policy::ToUInt(*this);
    }
};

//
//NOTE: we can switch between BigInt and TTMathBigInt here.
//
//typedef GenericBigInt<BigInt> TBigInt;
typedef GenericBigInt<TTMathBigInt> TBigInt;

template<>
struct std::formatter<TBigInt> : std::formatter<std::string_view>
{
private:
    static constexpr std::size_t max_spec_size = 256;
public:
    constexpr auto parse(std::format_parse_context& ctx)
    {
        auto begin = ctx.begin();
        const auto end = ctx.end();

        // Find closing '}' exactly for current replacement field.
        // Nested {} are possible, e.g. when dynamic width used.
        auto close = begin;
        std::size_t nested = 0;

        for (; close != end; ++close)
        {
            if (*close == '{')
            {
                ++nested;
            }
            else if (*close == '}')
            {
                if (nested == 0)
                    break;

                --nested;
            }
        }

        if (close == end)
            throw std::format_error("unterminated BigInt format specification");

        /*
         * L specificator can be located:
         *
         *   right before '}'
         *   right before 's'
         *   right before '?'
         *
         * Examples:
         *
         *   {:L}
         *   {:>20L}
         *   {:>20Ls}
         */
        auto localeSpecifier = close;

        if (localeSpecifier != begin)
        {
            --localeSpecifier;

            // when string presentation type, check symbol before it.
            if ((*localeSpecifier == 's' || *localeSpecifier == '?') && localeSpecifier != begin)
            {
                --localeSpecifier;
            }
        }

        // If no L found, we can call base parse() and finish.
        if (localeSpecifier == close || *localeSpecifier != 'L')
        {
            return std::formatter<std::string_view>::parse(ctx);
        }

        /*
         * Create a copy without L.
         *
         * Example:
         *
         *   >20L}  -> >20}
         *   *^20Ls} -> *^20s}
         */
        std::array<char, max_spec_size> filtered{};
        std::size_t size = 0;

        for (auto it = begin; it != close; ++it)
        {
            if (it == localeSpecifier)
                continue;

            if (size + 1 >= filtered.size())
            {
                throw std::format_error("TTMathBigInt format specification is too long");
            }

            filtered[size++] = *it;
        }

        // Base formatter expects to see closing '}'.
        filtered[size++] = '}';

        std::format_parse_context filteredContext{ std::string_view{filtered.data(), size} };

        // Base string formatter stores width, alingment, fill, precision and presentation type inside itself.
        const auto result = std::formatter<std::string_view>::parse(filteredContext);

        // make sure that base formatter went to '}'.
        if (result == filteredContext.end() || *result != '}')
        {
            throw std::format_error("invalid TTMathBigInt format specification");
        }

        // return iterator of ortiginal context instead of temporary.
        return close;
    }

    auto format(const TBigInt& value, std::format_context& ctx) const
    {
        const std::string digits = value.ToString();

        std::string result;
        result.reserve(digits.size() + digits.size() / 3);

        std::size_t firstDigit = 0;

        // Знак не должен участвовать в группировке цифр.
        if (!digits.empty() && (digits.front() == '-' || digits.front() == '+'))
        {
            result += digits.front();
            firstDigit = 1;
        }

        const std::size_t digitCount = digits.size() - firstDigit;

        for (std::size_t i = 0; i < digitCount; ++i)
        {
            if (i != 0 && (digitCount - i) % 3 == 0)
                result += ' ';

            result += digits[firstDigit + i];
        }

        /*
         * Здесь применяются настройки, которые базовый
         * formatter<string_view> сохранил в parse():
         *
         * - fill;
         * - align;
         * - width;
         * - precision;
         * - presentation type.
         */
        return std::formatter<std::string_view>::format(result, ctx);
    }
};
