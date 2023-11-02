# SPDX-FileCopyrightText: 2023 (c) Jack Hill <jackhill3103@gmail.com>
# SPDX-License-Identifier: GPL-2.0-or-later

include(ECMAddTests)

set(groupManagerTest_SOURCES
    groupmanagertest.cpp
)

ecm_add_test(${groupManagerTest_SOURCES}
    TEST_NAME "groupManagerTest"
    LINK_LIBRARIES Qt::Test ktcore
)

set(groupTreeModelTest_SOURCES
    grouptreemodeltest.cpp
)

ecm_add_test(${groupTreeModelTest_SOURCES}
    TEST_NAME "groupTreeModelTest"
    LINK_LIBRARIES Qt::Test ktcore
)
