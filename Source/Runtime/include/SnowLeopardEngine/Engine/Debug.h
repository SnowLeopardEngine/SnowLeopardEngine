#pragma once

#if !defined(ENABLE_EXCEPTION)
#define ENABLE_EXCEPTION 1
#endif

#if ENABLE_EXCEPTION
#define TRY try
#define CATCH \
    catch (const std::exception& e) \
    { \
        std::cerr << e.what() << std::endl; \
        return 1; \
    }
#else
#define TRY
#define CATCH
#endif