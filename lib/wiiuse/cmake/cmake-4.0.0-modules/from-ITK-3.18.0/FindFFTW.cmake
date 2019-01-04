## FFTW can be compiled and subsequently linked against
## various data types.
## There is a single set of include files, and then muttiple libraries,
## One for each type.  I.e. libfftw.a-->double, libfftwf.a-->float

## The following logic belongs in the individual package
## MARK_AS_ADVANCED(USE_FFTWD)
## OPTION(USE_FFTWD "Use double precision FFTW if found" ON)
## MARK_AS_ADVANCED(USE_FFTWF)
## OPTION(USE_FFTWF "Use single precision FFTW if found" ON)

IF(USE_FFTWD OR USE_FFTWF)

  SET(FFTW_INC_SEARCHPATH
    /sw/include
    /usr/include
    /usr/local/include
    /usr/include/fftw
    /usr/local/include/fftw
  )

  FIND_PATH(FFTW_INCLUDE_PATH fftw3.h ${FFTW_INC_SEARCHPATH})

  IF(FFTW_INCLUDE_PATH)
    SET(FFTW_INCLUDE ${FFTW_INCLUDE_PATH})
  ENDIF (FFTW_INCLUDE_PATH)

  IF(FFTW_INCLUDE)
    INCLUDE_DIRECTORIES( ${FFTW_INCLUDE})
  ENDIF(FFTW_INCLUDE)

  GET_FILENAME_COMPONENT(FFTW_INSTALL_BASE_PATH ${FFTW_INCLUDE_PATH} PATH)

  SET(FFTW_LIB_SEARCHPATH
    ${FFTW_INSTALL_BASE_PATH}/lib
    /usr/lib/fftw
    /usr/local/lib/fftw
  )

  IF(USE_FFTWD)
    MARK_AS_ADVANCED(FFTWD_LIB)
#   OPTION(FFTWD_LIB "The full path to the fftw3 library (including the library)" )
    FIND_LIBRARY(FFTWD_LIB fftw3 ${FFTW_LIB_SEARCHPATH}) #Double Precision Lib
    FIND_LIBRARY(FFTWD_THREADS_LIB fftw3_threads ${FFTW_LIB_SEARCHPATH}) #Double Precision Lib only if compiled with threads support

    IF(FFTWD_LIB)
      SET(FFTWD_FOUND 1)
      IF(FFTWD_THREADS_LIB)
        SET(FFTWD_LIB ${FFTWD_LIB} ${FFTWD_THREADS_LIB} )
      ENDIF(FFTWD_THREADS_LIB)
    ENDIF(FFTWD_LIB)
  ENDIF(USE_FFTWD)

  IF(USE_FFTWF)
    MARK_AS_ADVANCED(FFTWF_LIB)
#   OPTION(FFTWF_LIB "The full path to the fftw3f library (including the library)" )
    FIND_LIBRARY(FFTWF_LIB fftw3f ${FFTW_LIB_SEARCHPATH}) #Single Precision Lib
    FIND_LIBRARY(FFTWF_THREADS_LIB fftw3f_threads ${FFTW_LIB_SEARCHPATH}) #Single Precision Lib only if compiled with threads support

    IF(FFTWF_LIB)
      SET(FFTWF_FOUND 1)
      IF(FFTWF_THREADS_LIB)
        SET(FFTWF_LIB ${FFTWF_LIB} ${FFTWF_THREADS_LIB} )
      ENDIF(FFTWF_THREADS_LIB)
    ENDIF(FFTWF_LIB)
  ENDIF(USE_FFTWF)

ENDIF(USE_FFTWD OR USE_FFTWF)
