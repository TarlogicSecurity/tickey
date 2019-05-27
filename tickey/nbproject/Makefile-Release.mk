#
# Generated Makefile - do not edit!
#
# Edit the Makefile in the project folder instead (../Makefile). Each target
# has a -pre and a -post target defined where you can add customized code.
#
# This makefile implements configuration specific macros and targets.


# Environment
MKDIR=mkdir
CP=cp
GREP=grep
NM=nm
CCADMIN=CCadmin
RANLIB=ranlib
CC=gcc
CCC=g++
CXX=g++
FC=gfortran
AS=as

# Macros
CND_PLATFORM=GNU-Linux
CND_DLIB_EXT=so
CND_CONF=Release
CND_DISTDIR=dist
CND_BUILDDIR=build

# Include project Makefile
include Makefile

# Object Directory
OBJECTDIR=${CND_BUILDDIR}/${CND_CONF}/${CND_PLATFORM}

# Object Files
OBJECTFILES= \
	${OBJECTDIR}/keyctl.o \
	${OBJECTDIR}/keyrings.o \
	${OBJECTDIR}/krb_conf.o \
	${OBJECTDIR}/main.o \
	${OBJECTDIR}/proc_info/proc_info.o \
	${OBJECTDIR}/string_utils.o \
	${OBJECTDIR}/tickets.o \
	${OBJECTDIR}/traceter/p_trace.o \
	${OBJECTDIR}/traceter/traceter.o \
	${OBJECTDIR}/traceter/utils.o \
	${OBJECTDIR}/user_session.o


# C Compiler Flags
CFLAGS=

# CC Compiler Flags
CCFLAGS=
CXXFLAGS=

# Fortran Compiler Flags
FFLAGS=

# Assembler Flags
ASFLAGS=

# Link Libraries and Options
LDLIBSOPTIONS=

# Build Targets
.build-conf: ${BUILD_SUBPROJECTS}
	"${MAKE}"  -f nbproject/Makefile-${CND_CONF}.mk ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/tickey

${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/tickey: ${OBJECTFILES}
	${MKDIR} -p ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}
	${LINK.c} -o ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/tickey ${OBJECTFILES} ${LDLIBSOPTIONS}

${OBJECTDIR}/keyctl.o: keyctl.c
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.c) -O2 -Wall -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/keyctl.o keyctl.c

${OBJECTDIR}/keyrings.o: keyrings.c
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.c) -O2 -Wall -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/keyrings.o keyrings.c

${OBJECTDIR}/krb_conf.o: krb_conf.c
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.c) -O2 -Wall -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/krb_conf.o krb_conf.c

${OBJECTDIR}/main.o: main.c
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.c) -O2 -Wall -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/main.o main.c

${OBJECTDIR}/proc_info/proc_info.o: proc_info/proc_info.c
	${MKDIR} -p ${OBJECTDIR}/proc_info
	${RM} "$@.d"
	$(COMPILE.c) -O2 -Wall -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/proc_info/proc_info.o proc_info/proc_info.c

${OBJECTDIR}/string_utils.o: string_utils.c
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.c) -O2 -Wall -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/string_utils.o string_utils.c

${OBJECTDIR}/tickets.o: tickets.c
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.c) -O2 -Wall -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/tickets.o tickets.c

${OBJECTDIR}/traceter/p_trace.o: traceter/p_trace.c
	${MKDIR} -p ${OBJECTDIR}/traceter
	${RM} "$@.d"
	$(COMPILE.c) -O2 -Wall -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/traceter/p_trace.o traceter/p_trace.c

${OBJECTDIR}/traceter/traceter.o: traceter/traceter.c
	${MKDIR} -p ${OBJECTDIR}/traceter
	${RM} "$@.d"
	$(COMPILE.c) -O2 -Wall -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/traceter/traceter.o traceter/traceter.c

${OBJECTDIR}/traceter/utils.o: traceter/utils.c
	${MKDIR} -p ${OBJECTDIR}/traceter
	${RM} "$@.d"
	$(COMPILE.c) -O2 -Wall -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/traceter/utils.o traceter/utils.c

${OBJECTDIR}/user_session.o: user_session.c
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.c) -O2 -Wall -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/user_session.o user_session.c

# Subprojects
.build-subprojects:

# Clean Targets
.clean-conf: ${CLEAN_SUBPROJECTS}
	${RM} -r ${CND_BUILDDIR}/${CND_CONF}

# Subprojects
.clean-subprojects:

# Enable dependency checking
.dep.inc: .depcheck-impl

include .dep.inc
