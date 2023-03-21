# Copyright 2020 The Shaderc Authors. All rights reserved.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

ROOT_SHADERC_PATH := $(call my-dir)

include $(ROOT_SHADERC_PATH)/third_party/Android.mk
include $(ROOT_SHADERC_PATH)/libshaderc_util/Android.mk
include $(ROOT_SHADERC_PATH)/libshaderc/Android.mk

ALL_LIBS:=libglslang.a \
	libOGLCompiler.a \
	libOSDependent.a \
	libshaderc.a \
	libshaderc_util.a \
	libSPIRV.a \
	libHLSL.a \
	libSPIRV-Tools.a \
	libSPIRV-Tools-opt.a

SHADERC_HEADERS=shaderc.hpp shaderc.h env.h status.h visibility.h
SHADERC_HEADERS_IN_OUT_DIR=$(foreach H,$(SHADERC_HEADERS),$(NDK_APP_LIBS_OUT)/../include/shaderc/$(H))

define gen_libshaderc_header
$(NDK_APP_LIBS_OUT)/../include/shaderc/$(1) : \
		$(ROOT_SHADERC_PATH)/libshaderc/include/shaderc/$(1)
	$(call host-mkdir,$(NDK_APP_LIBS_OUT)/../include/shaderc)
	$(call host-cp,$(ROOT_SHADERC_PATH)/libshaderc/include/shaderc/$(1) \
		,$(NDK_APP_LIBS_OUT)/../include/shaderc/$(1))

endef

define gen_libshaderc

$(1)/combine.ar: $(addprefix $(1)/, $(ALL_LIBS))
	@echo "create libshaderc_combined.a" > $(1)/combine.ar
	$(foreach lib,$(ALL_LIBS),
		@echo "addlib $(lib)" >> $(1)/combine.ar
	)
	@echo "save" >> $(1)/combine.ar
	@echo "end" >> $(1)/combine.ar

$(1)/libshaderc_combined.a: $(addprefix $(1)/, $(ALL_LIBS)) $(1)/combine.ar
	@echo "[$(TARGET_ARCH_ABI)] Combine: libshaderc_combined.a <= $(ALL_LIBS)"
	@cd $(1) && $(TARGET_AR) -M < combine.ar && cd $(ROOT_SHADERC_PATH)
	@$(TARGET_STRIP) --strip-debug $(1)/libshaderc_combined.a

$(NDK_APP_LIBS_OUT)/$(APP_STL)/$(TARGET_ARCH_ABI)/libshaderc.a: \
		$(1)/libshaderc_combined.a
	$(call host-mkdir,$(NDK_APP_LIBS_OUT)/$(APP_STL)/$(TARGET_ARCH_ABI))
	$(call host-cp,$(1)/libshaderc_combined.a \
		,$(NDK_APP_LIBS_OUT)/$(APP_STL)/$(TARGET_ARCH_ABI)/libshaderc.a)

ifndef HEADER_TARGET
HEADER_TARGET=1
$(eval $(foreach H,$(SHADERC_HEADERS),$(call gen_libshaderc_header,$(H))))
endif

libshaderc_combined: \
	$(NDK_APP_LIBS_OUT)/$(APP_STL)/$(TARGET_ARCH_ABI)/libshaderc.a

endef

libshaderc_combined: $(SHADERC_HEADERS_IN_OUT_DIR)

$(eval $(call gen_libshaderc,$(TARGET_OUT)))
