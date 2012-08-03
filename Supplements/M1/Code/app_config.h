
/** Top Level Featureset
  */
#define OT_FEATURE(VAL)                 OT_FEATURE_##VAL
#define OT_FEATURE_BUFFER_SIZE          1024                                // Typically, must be at least 512 bytes    
#define OT_FEATURE_SERVER               ENABLED                             // need buffer queues

// not M2, but enable this just to get some basic stuff
#define OT_FEATURE_M2                   ENABLED                             // Mode 2 Featureset: Implemented


/// Logging Features (only available if C Server is enabled)
/// These control the things that are logged.  The way things are logged depends
/// on the implementation of the logging driver.
#define LOG_FEATURE(VAL)                (LOG_FEATURE_##VAL && OT_FEATURE_LOGGER)

/// Mode 2 Features:    
/// These are generally handled by the ISF settings files, but these defines 
/// can limit scope of the compilation if you are trying to optimize the build.
#define M2_FEATURE(VAL)                 (M2_FEATURESET && M2_FEATURE_##VAL)
#define M2_PARAM_MAXFRAME               255                                 // max length for q_init()
