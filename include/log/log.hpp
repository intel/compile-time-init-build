//
// Created by lvalenty on 8/18/2022.
//

#ifndef CIB_LOG_HPP
#define CIB_LOG_HPP

#ifndef LOG
#ifdef CIB_LOG_MIPI_CATALOG
#include <log/catalog/mipi_encoder.hpp>
#else
#include <log/fmt/logger.hpp>
#endif
#endif

#endif //CIB_LOG_HPP
