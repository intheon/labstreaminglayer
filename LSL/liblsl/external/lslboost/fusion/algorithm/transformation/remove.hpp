/*=============================================================================
    Copyright (c) 2001-2011 Joel de Guzman

    Distributed under the Boost Software License, Version 1.0. (See accompanying 
    file LICENSE_1_0.txt or copy at http://www.lslboost.org/LICENSE_1_0.txt)
==============================================================================*/
#if !defined(FUSION_REMOVE_07162005_0818)
#define FUSION_REMOVE_07162005_0818

#include <lslboost/fusion/view/filter_view/filter_view.hpp>
#include <lslboost/mpl/not.hpp>
#include <lslboost/type_traits/is_same.hpp>

namespace lslboost { namespace fusion
{
    namespace result_of
    {
        template <typename Sequence, typename T>
        struct remove
        {
            typedef filter_view<Sequence, mpl::not_<is_same<mpl::_, T> > > type;
        };
    }

    template <typename T, typename Sequence>
    inline typename result_of::remove<Sequence const, T>::type
    remove(Sequence const& seq)
    {
        typedef typename result_of::remove<Sequence const, T>::type result_type;
        return result_type(seq);
    }
}}

#endif

