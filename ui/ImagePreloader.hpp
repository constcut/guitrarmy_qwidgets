#ifndef IMAGEPRELOADER_H
#define IMAGEPRELOADER_H

#include <memory>
#include <string>
#include <unordered_map>
#include <QImage>


    namespace gtmy {


    class ImagePreloader
    {
    protected:
        std::unordered_map<std::string, std::unique_ptr<QImage>> _imageMap;
        bool _invertImages;

    public:
        void loadImage(std::string imageName);
        void loadImages();

        QImage* getImage(std::string imageName) const;

        void invertAll();

        void setInvert(bool toInvert) {_invertImages=toInvert;}

    public:
            static ImagePreloader& getInstance() {
                static ImagePreloader instance;
                return instance;
            }

    private:
            ImagePreloader(): _invertImages(false) {}
            ImagePreloader(const ImagePreloader& root) = delete;
            ImagePreloader& operator=(const ImagePreloader&) = delete;
    };

}

#endif // IMAGEPRELOADER_H
