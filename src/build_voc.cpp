/* GVAL
 * Copyright (c) 2014, Weipeng He <heweipeng@gmail.com>
 *
 * build_voc.cpp : Build BOW vocabulary
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General 
 * Public License as published by the Free Software 
 * Foundation; either version 2 of the License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will 
 * be useful, but WITHOUT ANY WARRANTY; without even the
 * implied warranty of MERCHANTABILITY or FITNESS FOR A
 * PARTICULAR PURPOSE. See the GNU General Public License
 * for more details.
 *
 * You should have received a copy of the GNU General 
 * Public License along with this program; if not, write
 * to the Free Software Foundation, Inc., 51 Franklin 
 * Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

extern "C" {
#include "gval_utils.h"
}

#include "gval_cv.hpp"

#include <unistd.h>
#include <assert.h>

#include <boost/filesystem.hpp>

using boost::filesystem::path;
using boost::filesystem::directory_iterator;

#include <opencv2/core/core.hpp>
#include <opencv2/features2d/features2d.hpp>

using cv::Mat;
using cv::BOWKMeansTrainer;
using cv::FlannBasedMatcher;
using cv::DMatch;

#include <vector>
using std::vector;

#define DESCRIPTORS_EXT ".desc"

int main(int argc, char** argv) {
  gval_debug_init();

  // get options
  int showhelp = 0;
  unsigned int n_cluster = 0;

  int opt;
  while ((opt = getopt(argc, argv, "k:h")) != -1) {
    switch (opt) {
      case 'h':
        showhelp = 1;
        break;
      case 'k':
        n_cluster = atoi(optarg);
        break;
      default:
        showhelp = 1;
        break;
    }
  }

  if (showhelp || n_cluster <= 0 || argc - optind < 2) {
    fprintf(stderr, "Usage: %s -k n_cluster input_dir output\n", argv[0]);
    return EXIT_SUCCESS;
  }

  path p(argv[optind]);
  if (!exists(p) || !is_directory(p)) {
    fprintf(stderr, "%s is not a directory\n", argv[optind]);
    return EXIT_FAILURE;
  }

  BOWKMeansTrainer bow(n_cluster);

  directory_iterator dir_end;
  for (directory_iterator i(p); i != dir_end; i++) {
    if (i->path().extension() == DESCRIPTORS_EXT) {
      FILE* in = fopen(i->path().c_str(), "r");
      assert(in);
      int counter = 0;
      int nempty = 0;
      Mat* desc = (Mat*) gval_read_cvmat(in);
      while (desc != NULL) {
        counter++;
        if (!desc->empty()) {
          nempty++;
          bow.add(desc->clone());
        }
        gval_free_cvmat(desc);
        desc = (Mat*) gval_read_cvmat(in);
      }
      fclose(in);
      fprintf(stderr, "Read from file %s (%d/%d)\n",
          i->path().c_str(), nempty, counter);
    }
  }

  fprintf(stderr, "Clustering (%d descriptors, %d clusters)...",
      bow.descripotorsCount(), n_cluster);
  Mat voc = bow.cluster();
  fprintf(stderr, " Done\n");

  fprintf(stderr, "Counting document frequency...");
  int* dfcounter = (int*) calloc(n_cluster, sizeof(int));
  vector<Mat> all = bow.getDescriptors();
  FlannBasedMatcher matcher;
  matcher.add(vector<Mat>(1, voc));
  for (vector<Mat>::const_iterator it = all.begin();
      it != all.end(); it++) {
    vector<int> ct(n_cluster, 0);
    vector<DMatch> matches;
    matcher.match(*it, matches);
    for (vector<DMatch>::const_iterator jt = matches.begin();
        jt != matches.end(); jt++) {
      assert(jt->trainIdx >= 0 && jt->trainIdx < n_cluster);
      ct[jt->trainIdx] = 1;
    }

    for (int j = 0; j < n_cluster; j++) {
      dfcounter[j] += ct[j];
    }
  }
  int dtotal = all.size();
  fprintf(stderr, " Done\n");

  FILE* out = fopen(argv[optind + 1], "w");
  assert(out);
  gval_write_cvmat(&voc, out);
  fwrite(dfcounter, sizeof(int), n_cluster, out);
  fwrite(&dtotal, sizeof(int), 1, out);

  // debug
  fprintf(stderr, "total:%d\n", dtotal);
  for (int j = 0; j < n_cluster; j++) {
    fprintf(stderr, "%d:%d\n", j, dfcounter[j]);
  }

  fclose(out);
  fprintf(stderr, "Written to %s\n", argv[optind + 1]);

  free(dfcounter);

  return EXIT_SUCCESS;
}

