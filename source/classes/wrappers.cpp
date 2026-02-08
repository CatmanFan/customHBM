#include "hbm/hbm.h"
#include "hbm/extern.h"

HBMElement *HBM_HBMElement_create(){
    HBMElement *c = (HBMElement *)malloc(sizeof(HBMElement));
    return c;
    //return (void*)c;
}
void HBM_HBMElement_destroy(HBMElement *c) { if (c != NULL) free (c); }
void HBM_HBMElement_update(HBMElement *c) { if (c != NULL) c->Update(); }
void HBM_HBMElement_draw(HBMElement *c) { if (c != NULL) c->Draw(); }

HBMImage *HBM_HBMImage_create() {
    HBMImage *c = (HBMImage *)malloc(sizeof(HBMImage));
    return c;
    //return (void*)c;
}
void HBM_HBMImage_destroy(HBMImage *c) { if (c != NULL) free (c); }
void HBM_HBMImage_draw(HBMImage *c) { if (c != NULL) c->Draw(); }

HBMHeader *HBM_HBMHeader_create() {
    HBMHeader *c = (HBMHeader *)malloc(sizeof(HBMHeader));
    return c;
    //return (void*)c;
}
void HBM_HBMHeader_destroy(HBMHeader *c) { if (c != NULL) free (c); }
void HBM_HBMHeader_draw(HBMHeader *c) { if (c != NULL) c->Draw(); }

HBMButtonMain *HBM_HBMButtonMain_create() {
    HBMButtonMain *c = new HBMButtonMain;
    return c;
    //return (void*)c;
}
void HBM_HBMButtonMain_destroy(HBMButtonMain *c) { if (c != NULL) free (c); }
void HBM_HBMButtonMain_draw(HBMButtonMain *c) { if (c != NULL) c->Draw(); }

HBMDialogButton *HBM_HBMDialogButton_create() {
    HBMDialogButton *c = new HBMDialogButton;
    return c;
    //return (void*)c;
}
void HBM_HBMDialogButton_destroy(HBMDialogButton *c) { if (c != NULL) free (c); }
void HBM_HBMDialogButton_draw(HBMDialogButton *c) { if (c != NULL) c->Draw(); }

HBMDialog *HBM_HBMDialog_create() {
    HBMDialog *c = new HBMDialog;
    return c;
    //return (void*)c;
}
void HBM_HBMDialog_destroy(HBMDialog *c) { if (c != NULL) free (c); }
void HBM_HBMDialog_draw(HBMDialog *c) { if (c != NULL) c->Draw(); }

HBMPointerImage *HBM_HBMPointerImage_create() {
    HBMPointerImage *c = new HBMPointerImage;
    return c;
    //return (void*)c;
}
void HBM_HBMPointerImage_destroy(HBMPointerImage *c) { if (c != NULL) free (c); }
void HBM_HBMPointerImage_draw(HBMPointerImage *c) { if (c != NULL) c->Draw(); }

HBMRemoteDataSprite *HBM_HBMRemoteDataSprite_create() {
    HBMRemoteDataSprite *c = new HBMRemoteDataSprite;
    return c;
    //return (void*)c;
}
void HBM_HBMRemoteDataSprite_destroy(HBMRemoteDataSprite *c) { if (c != NULL) free (c); }
void HBM_HBMRemoteDataSprite_draw(HBMRemoteDataSprite *c, int X, int Y) { c->Draw(X, Y); }