#include "detail/set_details.hpp"

#ifndef CIB_SET_HPP
#define CIB_SET_HPP


namespace cib {
    template<
        typename LhsTuple,
        typename RhsTuple,
        typename MetaFunc = self_type>
    [[nodiscard]] constexpr auto set_union(
        LhsTuple lhs,
        RhsTuple rhs,
        MetaFunc meta_func = self_type{}
    ) {
        return set_operation_impl(detail::set_operation_algorithm<true, true, true>{}, meta_func, lhs, rhs);
    }

    template<
        typename LhsTuple,
        typename RhsTuple,
        typename MetaFunc = self_type>
    [[nodiscard]] constexpr auto set_intersection(
        LhsTuple lhs,
        RhsTuple rhs,
        MetaFunc meta_func = self_type{}
    ) {
        return set_operation_impl(detail::set_operation_algorithm<false, true, false>{}, meta_func, lhs, rhs);
    }

    template<
        typename LhsTuple,
        typename RhsTuple,
        typename MetaFunc = self_type>
    [[nodiscard]] constexpr auto set_difference(
        LhsTuple lhs,
        RhsTuple rhs,
        MetaFunc meta_func = self_type{}
    ) {
        return set_operation_impl(detail::set_operation_algorithm<true, false, false>{}, meta_func, lhs, rhs);
    }

    template<
        typename LhsTuple,
        typename RhsTuple,
        typename MetaFunc = self_type>
    [[nodiscard]] constexpr auto set_symmetric_difference(
        LhsTuple lhs,
        RhsTuple rhs,
        MetaFunc meta_func = self_type{}
    ) {
        return set_operation_impl(detail::set_operation_algorithm<true, false, true>{}, meta_func, lhs, rhs);
    }

    template<
        typename IndexType,
        IndexType... Indexes,
        typename Operation>
    [[nodiscard]] constexpr auto make_tuple_from_op(
        std::integer_sequence<IndexType, Indexes...>,
        Operation op
    ) {
        return make_tuple(op(std::integral_constant<IndexType, Indexes>{})...);
    }

    template<
        typename IndexType,
        IndexType Size,
        typename Operation>
    [[nodiscard]] constexpr auto make_tuple(
        std::integral_constant<IndexType, Size>,
        Operation op
    ) {
        return make_tuple_from_op(std::make_integer_sequence<IndexType, Size>{}, op);
    }

    template<typename MetaFunc, typename Tuple>
    struct create_demux_tags_t;

    template<typename MetaFunc, typename... TupleElems>
    struct create_demux_tags_t<MetaFunc, cib::tuple_impl<TupleElems...>> {
        constexpr static auto invoke() {
            constexpr auto value = [](){
                constexpr auto const_type_names = cib::detail::create_type_names<MetaFunc, typename TupleElems::value_type...>(0);
                auto type_names = const_type_names;

                // assign all type_names with the same name the same src id
                auto prev_name = type_names.front();
                std::size_t name_dst_index = 0;
                for (auto & name : type_names) {
                    if (name != prev_name) {
                        prev_name = name;
                        name_dst_index += 1;
                    }
                    name.src = name_dst_index;
                }

                return type_names;
            }();

            return value;
        }
    };

    template<typename MetaFunc, typename... TupleElems>
    constexpr static auto create_demux_tags(MetaFunc, cib::tuple_impl<TupleElems...>){
        constexpr auto const value = [](){
            constexpr auto const_type_names = cib::detail::create_type_names<MetaFunc, typename TupleElems::value_type...>(0);
            auto type_names = const_type_names;

            // assign all type_names with the same name the same src id
            auto prev_name = type_names.front();
            std::size_t name_dst_index = 0;
            for (auto & name : type_names) {
                if (name != prev_name) {
                    prev_name = name;
                    name_dst_index += 1;
                }
                name.src = name_dst_index;
            }

            return type_names;
        }();

        return value;
    }

    /**
     * De-multiplex a tuple (tn) into a tuple of tuples grouped by a meta function (meta_func).
     */
    template<
        typename MetaFunc,
        typename Tuple>
    [[nodiscard]] constexpr auto demux(
        MetaFunc,
        Tuple t
    ) {
        using detail::size_;

        // workaround for gcc bug
        #if not (defined(__clang__)) && (defined(__GNUC__) || defined(__GNUG__))
            #define tags create_demux_tags_t<MetaFunc, Tuple>::invoke()
        #else
            constexpr auto tags = create_demux_tags_t<MetaFunc, Tuple>::invoke();
        #endif

        constexpr std::size_t num_bins =
            tags.empty() ? 0 : (tags.back().src + 1);

        // FIXME: this should use generic algorithms
        // the number of elements in each bin in 'tags'
        constexpr std::array<std::size_t, num_bins> bin_size = [&](){
            std::array<std::size_t, num_bins> bin_size_{};

            std::size_t begin = 0;
            std::size_t end = begin;
            for (std::size_t bin_index = 0; bin_index < num_bins; bin_index++) {
                while (end != tags.size() && tags[end].src == bin_index) {
                    end++;
                }
                bin_size_[bin_index] = end - begin;
                begin = end;
            }

            return bin_size_;
        }();


        // FIXME: this should use generic algorithms
        // the offset of each bin in 'tags'
        constexpr std::array<std::size_t, num_bins> bin_offset = [&](){
            std::array<std::size_t, num_bins> bin_offset_{};

            std::size_t begin = 0;
            std::size_t end = begin;
            for (std::size_t bin_index = 0; bin_index < num_bins; bin_index++) {
                while (end != tags.size() && tags[end].src == bin_index) {
                    end++;
                }
                bin_offset_[bin_index] = begin;
                begin = end;
            }

            return bin_offset_;
        }();

        return make_tuple(size_<num_bins>, [&](auto bin_index){
            return make_tuple(size_<bin_size[bin_index]>, [&](auto offset_into_bin){
                constexpr int tuple_index =
                    tags[bin_offset[bin_index] + offset_into_bin].index;

                return t.get(index_<tuple_index>);
            });
        });

        #undef tags
    }
}

#endif //CIB_SET_HPP
