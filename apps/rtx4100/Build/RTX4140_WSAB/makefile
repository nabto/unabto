###############################################################################
#
# Node definition and name of this node. MANDATORY
#
###############################################################################
NODE=Projects\Amelie\COLApps\Apps\Nabto\unabto\demo\rtx4100\Build\RTX4140_WSAB
NODE_NAME=Nabto

###############################################################################
#
# System Configuration section.
#
###############################################################################
COLASCRIPTS=..\..\..\..\..\..\..\Scripts
MODULE=RTX4140
COMMONOPTIONS:=$(COMMONOPTIONS) -I../../../../src -I../../ -DAPP_WIFI_NUMBER_OF_APS=1 -DAPP_SHELL_ENABLED=1 -DAPP_SHELL_CONFIG_CMDS_ONLY=1
include $(COLASCRIPTS)\Cola.gmk

# End of file.
