
#include "gft_cimage.h"
#include <vector>

namespace gft
{
  namespace CImage
  {

    sCImage *Create(int ncols, int nrows)
    {
      sCImage *cimg = NULL;
      int i;
      cimg = (sCImage *)calloc(1, sizeof(sCImage));
      for (i = 0; i < 3; i++)
        cimg->C[i] = Image32::Create(ncols, nrows);
      return (cimg);
    }

    sCImage *Create(sCImage *cimg)
    {
      return Create(cimg->C[0]->ncols, cimg->C[0]->nrows);
    }

    sCImage *Create(sImage32 *img)
    {
      return Create(img->ncols, img->nrows);
    }

    void Destroy(sCImage **cimg)
    {
      sCImage *tmp;
      int i;

      tmp = *cimg;
      if (tmp != NULL)
      {
        for (i = 0; i < 3; i++)
          Image32::Destroy(&(tmp->C[i]));
        free(tmp);
        *cimg = NULL;
      }
    }

    sCImage *RandomColorize(sImage32 *img)
    {
      sCImage *cimg = NULL;
      int p, n, Imax, i, color;
      int *colormap = NULL;

      n = img->ncols * img->nrows;
      cimg = gft::CImage::Create(img);
      Imax = gft::Image32::GetMaxVal(img);
      colormap = gft::AllocIntArray(Imax + 1);
      colormap[0] = 0x000000;
      for (i = 1; i <= Imax; i++)
        colormap[i] = gft::Color::RandomColor();

      for (p = 0; p < n; p++)
      {
        color = colormap[img->data[p]];
        (cimg->C[0])->data[p] = gft::Color::Channel0(color);
        (cimg->C[1])->data[p] = gft::Color::Channel1(color);
        (cimg->C[2])->data[p] = gft::Color::Channel2(color);
      }
      gft::FreeIntArray(&colormap);
      return cimg;
    }

    gft::sCImage *ColorizeByAverageColor(gft::sCImage *orig, gft::sImage32 *label) {
      int n = label->ncols * label->nrows;
      int Imax = gft::Image32::GetMaxVal(label);
      
      std::vector<long> sum_r(Imax + 1, 0);
      std::vector<long> sum_g(Imax + 1, 0);
      std::vector<long> sum_b(Imax + 1, 0);
      std::vector<int> count(Imax + 1, 0);
      
      for (int p = 0; p < n; p++) {
        int lbl = label->data[p];
        sum_r[lbl] += orig->C[0]->data[p];
        sum_g[lbl] += orig->C[1]->data[p];
        sum_b[lbl] += orig->C[2]->data[p];
        count[lbl]++;
      }
      
      std::vector<int> avg_r(Imax + 1, 0);
      std::vector<int> avg_g(Imax + 1, 0);
      std::vector<int> avg_b(Imax + 1, 0);
      
      for (int i = 1; i <= Imax; i++) {
        if (count[i] > 0) {
          avg_r[i] = sum_r[i] / count[i];
          avg_g[i] = sum_g[i] / count[i];
          avg_b[i] = sum_b[i] / count[i];
        }
      }
      
      gft::sCImage *cimg = gft::CImage::Create(label);
      for (int p = 0; p < n; p++) {
        int lbl = label->data[p];
        cimg->C[0]->data[p] = avg_r[lbl];
        cimg->C[1]->data[p] = avg_g[lbl];
        cimg->C[2]->data[p] = avg_b[lbl];
      }
      
      return cimg;
    }


    sCImage *Read(char *filename)
    {
      sCImage *cimg = NULL;
      FILE *fp = NULL;
      char type[10];
      int t, i, ncols, nrows, n;
      char z[256];
      int l;
      sImage32 *img;

      l = strlen(filename);
      if (strcasecmp(filename + l - 3, "pgm") == 0)
      {
        img = gft::Image32::Read(filename);
        cimg = Create(img->ncols, img->nrows);
        for (i = 0; i < img->n; i++)
        {
          cimg->C[0]->data[i] = img->data[i];
          cimg->C[1]->data[i] = img->data[i];
          cimg->C[2]->data[i] = img->data[i];
        }
        gft::Image32::Destroy(&img);
        return cimg;
      }

      fp = fopen(filename, "rb");
      if (fp == NULL)
      {
        fprintf(stderr, "Cannot open %s\n", filename);
        exit(-1);
      }
      fscanf(fp, "%s\n", type);
      if ((strcmp(type, "P6") == 0))
      {
        gft::NCFgets(z, 255, fp);
        t = sscanf(z, "%d %d\n", &ncols, &nrows);
        if (t == EOF || t < 2)
        {
          gft::NCFgets(z, 255, fp);
          sscanf(z, "%d %d\n", &ncols, &nrows);
        }
        n = ncols * nrows;
        gft::NCFgets(z, 255, fp);
        sscanf(z, "%d\n", &i);
        cimg = Create(ncols, nrows);
        for (i = 0; i < n; i++)
        {
          cimg->C[0]->data[i] = fgetc(fp);
          cimg->C[1]->data[i] = fgetc(fp);
          cimg->C[2]->data[i] = fgetc(fp);
        }
        fclose(fp);
      }
      else
      {
        fprintf(stderr, "Input image must be P6\n");
        exit(-1);
      }

      return (cimg);
    }

    void Write(sCImage *cimg, char *filename)
    {
      FILE *fp;
      int i, n;

      fp = fopen(filename, "w");
      fprintf(fp, "P6\n");
      fprintf(fp, "%d %d\n", cimg->C[0]->ncols, cimg->C[0]->nrows);
      fprintf(fp, "255\n");
      n = cimg->C[0]->ncols * cimg->C[0]->nrows;
      for (i = 0; i < n; i++)
      {
        fputc(cimg->C[0]->data[i], fp);
        fputc(cimg->C[1]->data[i], fp);
        fputc(cimg->C[2]->data[i], fp);
      }
      fclose(fp);
    }

    sCImage *Clone(sCImage *cimg)
    {
      sCImage *imgc;
      int i;

      imgc = (sCImage *)calloc(1, sizeof(sCImage));
      if (imgc == NULL)
      {
        gft::Error((char *)MSG1, (char *)"CImage::Clone");
      }
      for (i = 0; i < 3; i++)
        imgc->C[i] = Image32::Clone(cimg->C[i]);
      return imgc;
    }

    sCImage *Clone(sImage32 *img)
    {
      sCImage *imgc;
      sImage32 *img8;
      int i;
      if (GetMinVal(img) < 0 ||
          GetMaxVal(img) > 255)
        img8 = Image32::ConvertToNbits(img, 8);
      else
        img8 = Image32::Clone(img);

      imgc = (sCImage *)calloc(1, sizeof(sCImage));
      if (imgc == NULL)
      {
        gft::Error((char *)MSG1, (char *)"CImage::Clone");
      }
      imgc->C[0] = img8;
      for (i = 1; i < 3; i++)
        imgc->C[i] = Image32::Clone(img8);
      return imgc;
    }

    sCImage *Clone(sImage32 *img, int Imin, int Imax)
    {
      sCImage *imgc;
      sImage32 *img8;
      int i;

      img8 = Image32::Create(img);

      if (Imin != Imax)
      {
        for (i = 0; i < img->n; i++)
        {
          if ((img->data[i] != INT_MIN) && (img->data[i] != INT_MAX))
          {
            img8->data[i] = (int)(((float)255 * (float)(img->data[i] - Imin)) /
                                  (float)(Imax - Imin));
          }
          else
          {
            if (img->data[i] == INT_MIN)
              img8->data[i] = 0;
            else
              img8->data[i] = 255;
          }
        }
      }

      imgc = (sCImage *)calloc(1, sizeof(sCImage));
      if (imgc == NULL)
      {
        gft::Error((char *)MSG1, (char *)"CImage::Clone");
      }
      imgc->C[0] = img8;
      for (i = 1; i < 3; i++)
        imgc->C[i] = Image32::Clone(img8);
      return imgc;
    }

    sCImage *Clone(sCImage *cimg, Pixel l, Pixel h)
    {
      sCImage *sub;
      sub = (sCImage *)calloc(1, sizeof(sCImage));
      if (sub == NULL)
      {
        gft::Error((char *)MSG1, (char *)"CImage::Clone");
      }
      sub->C[0] = gft::Image32::Clone(cimg->C[0], l, h);
      sub->C[1] = gft::Image32::Clone(cimg->C[1], l, h);
      sub->C[2] = gft::Image32::Clone(cimg->C[2], l, h);
      return sub;
    }

    void Copy(sCImage *cimg, sCImage *sub, Pixel l)
    {
      gft::Image32::Copy(cimg->C[0], sub->C[0], l);
      gft::Image32::Copy(cimg->C[1], sub->C[1], l);
      gft::Image32::Copy(cimg->C[2], sub->C[2], l);
    }

    void Set(sCImage *cimg, int r, int g, int b)
    {
      Image32::Set(cimg->C[0], r);
      Image32::Set(cimg->C[1], g);
      Image32::Set(cimg->C[2], b);
    }

    void Set(sCImage *cimg, int color)
    {
      Image32::Set(cimg->C[0], gft::Color::Channel0(color));
      Image32::Set(cimg->C[1], gft::Color::Channel1(color));
      Image32::Set(cimg->C[2], gft::Color::Channel2(color));
    }

    sCImage *ColorizeLabel(sImage32 *label)
    {
      sCImage *cimg;
      int n, p, l, Lmax = Image32::GetMaxVal(label);
      int *R, *G, *B;

      cimg = Create(label->ncols, label->nrows);
      R = gft::AllocIntArray(Lmax + 1);
      G = gft::AllocIntArray(Lmax + 1);
      B = gft::AllocIntArray(Lmax + 1);
      gft::RandomSeed();
      for (l = 0; l <= Lmax; l++)
      {
        R[l] = gft::RandomInteger(0, 255);
        G[l] = gft::RandomInteger(0, 255);
        B[l] = gft::RandomInteger(0, 255);
      }
      n = label->ncols * label->nrows;
      for (p = 0; p < n; p++)
      {
        l = label->data[p];
        cimg->C[0]->data[p] = R[l];
        cimg->C[1]->data[p] = G[l];
        cimg->C[2]->data[p] = B[l];
      }
      free(R);
      free(G);
      free(B);
      return cimg;
    }

    sCImage *RGB2Lab(sCImage *cimg)
    {
      sCImage *cimg_lab;
      double l, a, b;
      int p, n;
      n = cimg->C[0]->n;
      cimg_lab = Create(cimg->C[0]->ncols, cimg->C[0]->nrows);

      // pragma omp parallel for private(l,a,b)
      for (p = 0; p < n; p++)
      {
        gft::Color::RGB2Lab(cimg->C[0]->data[p],
                            cimg->C[1]->data[p],
                            cimg->C[2]->data[p],
                            l, a, b);
        cimg_lab->C[0]->data[p] = ROUND(l);
        cimg_lab->C[1]->data[p] = ROUND(a);
        cimg_lab->C[2]->data[p] = ROUND(b);
      }
      return cimg_lab;
    }

    sImage32 *Lightness(sCImage *cimg)
    {
      sImage32 *img;
      int p, n, max, min;
      n = cimg->C[0]->n;
      img = Image32::Create(cimg->C[0]->ncols, cimg->C[0]->nrows);
      for (p = 0; p < n; p++)
      {
        max = MAX(cimg->C[0]->data[p],
                  MAX(cimg->C[1]->data[p], cimg->C[2]->data[p]));
        min = MIN(cimg->C[0]->data[p],
                  MIN(cimg->C[1]->data[p], cimg->C[2]->data[p]));
        img->data[p] = ROUND((max + min) / 2.0);
      }
      return img;
    }

    /*The luminosity method works best overall and is the
      default method used if you ask GIMP to change an image
      from RGB to grayscale */
    /*It averages the values, but it forms a weighted average
      to account for human perception.
      We’re more sensitive to green than other colors, so green
      is weighted most heavily. */
    sImage32 *Luminosity(sCImage *cimg)
    {
      sImage32 *img;
      int p, n;
      n = cimg->C[0]->n;
      img = Image32::Create(cimg->C[0]->ncols, cimg->C[0]->nrows);
      for (p = 0; p < n; p++)
      {
        img->data[p] = ROUND(0.21 * cimg->C[0]->data[p] +
                             0.72 * cimg->C[1]->data[p] +
                             0.07 * cimg->C[2]->data[p]);
      }
      return img;
    }

    void MBB(sCImage *cimg, int bkgcolor, Pixel *l, Pixel *h)
    {
      int ncols, nrows, p, i, j, color, R, G, B;
      ncols = cimg->C[0]->ncols;
      nrows = cimg->C[0]->nrows;
      l->x = ncols - 1;
      l->y = nrows - 1;
      h->x = 0;
      h->y = 0;
      for (i = 0; i < nrows; i++)
      {
        for (j = 0; j < ncols; j++)
        {
          p = j + i * ncols;
          R = cimg->C[0]->data[p];
          G = cimg->C[1]->data[p];
          B = cimg->C[2]->data[p];
          color = gft::Color::Triplet(R, G, B);
          if (color != bkgcolor)
          {
            if (j < l->x)
              l->x = j;
            if (i < l->y)
              l->y = i;
            if (j > h->x)
              h->x = j;
            if (i > h->y)
              h->y = i;
          }
        }
      }
    }

    sCImage *SubImage(sCImage *cimg, Pixel l, Pixel h)
    {
      int i, j, p, q, ncols, nrows;
      sCImage *sub = NULL;
      ncols = cimg->C[0]->ncols;
      nrows = cimg->C[0]->nrows;
      if (l.x >= 0 && l.y >= 0 && h.x < ncols && h.y < nrows && l.x <= h.x && l.y <= h.y)
      {
        sub = Create(h.x - l.x + 1, h.y - l.y + 1);
        p = 0;
        for (i = l.y; i <= h.y; i++)
        {
          for (j = l.x; j <= h.x; j++)
          {
            q = j + i * ncols;
            sub->C[0]->data[p] = cimg->C[0]->data[q];
            sub->C[1]->data[p] = cimg->C[1]->data[q];
            sub->C[2]->data[p] = cimg->C[2]->data[q];
            p++;
          }
        }
      }
      return sub;
    }

    sCImage *AddFrame(sCImage *cimg, int sz, int r, int g, int b)
    {
      sCImage *fcimg;
      fcimg = (sCImage *)calloc(1, sizeof(sCImage));
      if (fcimg == NULL)
        gft::Error((char *)MSG1, (char *)"CImage::AddFrame");
      fcimg->C[0] = gft::Image32::AddFrame(cimg->C[0], sz, r);
      fcimg->C[1] = gft::Image32::AddFrame(cimg->C[1], sz, g);
      fcimg->C[2] = gft::Image32::AddFrame(cimg->C[2], sz, b);
      return fcimg;
    }

    sCImage *RemFrame(sCImage *cimg, int sz)
    {
      sCImage *fcimg;
      fcimg = (sCImage *)calloc(1, sizeof(sCImage));
      if (fcimg == NULL)
        gft::Error((char *)MSG1, (char *)"CImage::AddFrame");
      fcimg->C[0] = gft::Image32::RemFrame(cimg->C[0], sz);
      fcimg->C[1] = gft::Image32::RemFrame(cimg->C[1], sz);
      fcimg->C[2] = gft::Image32::RemFrame(cimg->C[2], sz);
      return fcimg;
    }

    void DrawRectangle(sCImage *cimg,
                       int x1, int y1,
                       int x2, int y2, int color)
    {
      DrawRectangle(cimg->C[0],
                    x1, y1, x2, y2, gft::Color::Channel0(color));
      DrawRectangle(cimg->C[1],
                    x1, y1, x2, y2, gft::Color::Channel1(color));
      DrawRectangle(cimg->C[2],
                    x1, y1, x2, y2, gft::Color::Channel2(color));
    }

    void DrawLineDDA(sCImage *cimg,
                     int x1, int y1,
                     int xn, int yn, int color)
    {
      DrawLineDDA(cimg->C[0],
                  x1, y1, xn, yn, gft::Color::Channel0(color));
      DrawLineDDA(cimg->C[1],
                  x1, y1, xn, yn, gft::Color::Channel1(color));
      DrawLineDDA(cimg->C[2],
                  x1, y1, xn, yn, gft::Color::Channel2(color));
    }

    void DrawCircle(sCImage *cimg,
                    int x1, int y1,
                    float r,
                    int color)
    {
      DrawCircle(cimg->C[0],
                 x1, y1, r,
                 gft::Color::Channel0(color));
      DrawCircle(cimg->C[1],
                 x1, y1, r,
                 gft::Color::Channel1(color));
      DrawCircle(cimg->C[2],
                 x1, y1, r,
                 gft::Color::Channel2(color));
    }

    sCImage *Scale(sCImage *cimg, float Sx, float Sy,
                   gft::InterpolationType interpolation)
    {
      sCImage *imgs;
      int i;

      imgs = (sCImage *)calloc(1, sizeof(sCImage));
      if (imgs == NULL)
      {
        gft::Error((char *)MSG1, (char *)"CImage::Scale");
      }
      for (i = 0; i < 3; i++)
        imgs->C[i] = Image32::Scale(cimg->C[i], Sx, Sy, interpolation);
      return imgs;
    }

  } /*end CImage namespace*/
} /*end gft namespace*/
