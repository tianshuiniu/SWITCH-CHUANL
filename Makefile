#===============================================================================
# libnx (Nintendo Switch) 构建文件 —— xCloud 串流客户端（第一阶段：鉴权 + REST）
#
# 依赖（用 devkitPro 的 pacman 安装）：
#   (dkp-)pacman -S switch-dev switch-curl switch-mbedtls
#   （switch-dev 已含 libnx / devkitA64 工具链）
#
# 用法：
#   export DEVKITPRO=/opt/devkitpro   # Windows 下由 devkitPro 安装器自动设置
#   make                              # 产出 xcloud_switch.nro
#   make clean
#
# 说明：
#   - 本 Makefile 仅编译 src/ 下的源文件（含 switch_main.cpp 入口），
#     桌面端 main.cpp 不会被编入 NRO。
#   - 鉴权 / 信令逻辑与桌面端完全共用（src/auth.cpp、src/stream.cpp）。
#   - 第二阶段（WebRTC 媒体：移植 libdatachannel + FFmpeg 解码）接入后，
#     只需在 LIBS 补 -ldatachannel -lavformat -lavcodec -lswscale 等。
#===============================================================================

ifeq ($(strip $(DEVKITPRO)),)
$(error "请在环境中设置 DEVKITPRO。Windows: devkitPro 安装器已自动设置；Linux/macOS: export DEVKITPRO=/opt/devkitpro")
endif

include $(DEVKITPRO)/libnx/switch_rules

#-------------------------------------------------------------------------------
# 产物名（生成的 .nro 不带扩展名）
#-------------------------------------------------------------------------------
NAME		:=	xcloud_switch
BUILD		:=	build
#-------------------------------------------------------------------------------
# 源码目录（仅 src/；桌面 main.cpp 在根目录，不编入 NRO）
#-------------------------------------------------------------------------------
SRCS		:=	src
#-------------------------------------------------------------------------------
# 头文件搜索：src 同级、portlibs/libnx 自带、include/、third_party/（含 vendored nlohmann）
#-------------------------------------------------------------------------------
INCS		:=	$(PORTLIBS) $(LIBNX)
export INCLUDE := $(foreach dir,$(SRCS),-I$(CURDIR)/$(dir)) \
                  $(foreach dir,$(INCS),-I$(dir)/include) \
                  -I$(CURDIR)/include -I$(CURDIR)/third_party
#-------------------------------------------------------------------------------
# 链接库（依赖顺序：被依赖者在前）
#   curl -> mbedtls -> mbedx509 -> mbedcrypto ；z 为 curl 压缩支持 ；nx 为系统库
#   第二阶段接入 WebRTC/解码时，在此追加 -ldatachannel -lavcodec -lswscale ...
#-------------------------------------------------------------------------------
LIBS		:=	-lcurl -lmbedtls -lmbedx509 -lmbedcrypto -lz -lnx
#-------------------------------------------------------------------------------
# NRO 元数据
#-------------------------------------------------------------------------------
APP_TITLE	:=	XCloud Switch
APP_AUTHOR	:=	research
APP_VERSION	:=	0.1
APP_ICON	:=	icon.jpg
#-------------------------------------------------------------------------------
# 编译选项
#-------------------------------------------------------------------------------
CXXFLAGS	+=	-D__SWITCH__ -std=c++17 -O2 -ffunction-sections
CFLAGS		+=	-D__SWITCH__
NROFLAGS	+=	--icon=$(APP_ICON)

#===============================================================================
# 以下为标准 devkitPro 构建目录切换块（无需修改）
#===============================================================================
ifneq ($(BUILD),$(notdir $(CURDIR)))
export LD	:=	$(CXX)
export OUTPUT	:=	$(CURDIR)/$(NAME)
export DEPSDIR	:=	$(CURDIR)/$(BUILD)
export LIBPATHS	:=	$(foreach dir,$(INCS),-L$(dir)/lib)
export VPATH	:=	$(foreach dir,$(SRCS),$(CURDIR)/$(dir))
export NROFLAGS	+=	--nacp=$(CURDIR)/$(NAME).nacp
CPPFILES	:=	$(foreach dir,$(SRCS),$(notdir $(wildcard $(dir)/*.cpp)))
export OFILES	:=	$(CPPFILES:.cpp=.o)
.PHONY: $(BUILD)
all: $(BUILD)
$(BUILD):
	@mkdir -p $@
	@$(MAKE) --no-print-directory -C $(BUILD) -f $(CURDIR)/Makefile
clean:
	@rm -rf $(BUILD) $(NAME).nro $(NAME).nacp $(NAME).elf
else
all: $(OUTPUT).nro
$(OUTPUT).nro: $(OUTPUT).elf $(OUTPUT).nacp
$(OUTPUT).elf: $(OFILES)
DEPENDS := $(OFILES:.o=.d)
-include $(DEPENDS)
endif
