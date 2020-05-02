/*                            M A i N . C P P
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 * SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER
 * RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF
 * CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */
/** @file main.cpp
 *
 * The old PrintTitle program:
 *     Tests the BRL-CAD MOOSE library and the build system
 *
 */

#include <iostream>

#include <brlcad/Database/ConstDatabase.h>
#include <brlcad/memory.h>


int main
(
    int   argc,
    char* argv[]
) {
    int ret = 0;

    if (argc < 2) {
        std::cout << "Usage: " << argv[0] << " <BRL-CAD Database>" << std::endl;
        ret = 1;
    }
    else {
        try {
            BRLCAD::ConstDatabase database;

            if (database.Load(argv[1]))
                std::cout << database.Title() << std::endl;
            else {
                std::cerr << "Could not load file: " << argv[1] << std::endl;
                ret = 2;
            }
        }
        catch(BRLCAD::bad_alloc& e) {
            std::cerr << "Out of memory in: " << e.what() << std::endl;
            ret = 3;
        }
    }

    return ret;
}
