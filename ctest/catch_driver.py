#! python

################################################################################
#                              INTEL CONFIDENTIAL
#   Copyright(C) 2015 Intel Corporation. All Rights Reserved.
#   The source code contained  or  described herein and all documents related to
#   the source code ("Material") are owned by Intel Corporation or its suppliers
#   or licensors.  Title to the  Material remains with  Intel Corporation or its
#   suppliers and licensors. The Material contains trade secrets and proprietary
#   and  confidential  information of  Intel or its suppliers and licensors. The
#   Material  is  protected  by  worldwide  copyright  and trade secret laws and
#   treaty  provisions. No part of the Material may be used, copied, reproduced,
#   modified, published, uploaded, posted, transmitted, distributed or disclosed
#   in any way without Intel's prior express written permission.
#   No license  under any  patent, copyright, trade secret or other intellectual
#   property right is granted to or conferred upon you by disclosure or delivery
#   of the Materials,  either expressly, by implication, inducement, estoppel or
#   otherwise.  Any  license  under  such  intellectual property  rights must be
#   express and approved by Intel in writing.
#
################################################################################

import subprocess
import sys
import os

def main(test_executable, reporter, output_dir):
    args = []

    if reporter:
        args += ["-r", reporter]

    if output_dir:
        if not os.path.exists(output_dir):
            os.makedirs(output_dir)

        exe_basename = os.path.basename(test_executable)
        exe_basename = os.path.splitext(exe_basename)[0]
        output_path = os.path.join(output_dir, "testLog{}.txt".format(exe_basename))
        output_arg = ["-o", output_path]

        args += output_arg

    status = subprocess.call([test_executable] + args)
    sys.exit(status)

if __name__ == '__main__':
    main(sys.argv[1],
         os.environ.get('CATCH_REPORTER', None),
         os.environ.get('CATCH_OUTPUT_DIR', None))
