#ifndef IMAGEPRELOADER_H
#define IMAGEPRELOADER_H

#include <memory>
#include <string>
#include <unordered_map>
#include <QImage>


class ImagePreloader
{
protected:
    std::unordered_map<std::string, std::unique_ptr<QImage>> imageMap;
    bool inv;

public:
    void loadImage(std::string imageName);
    void loadImages();

    QImage *getImage(std::string imageName);

    void invertAll();

    void setInvert(bool toInvert) {inv=toInvert;}

public:
        static ImagePreloader& getInstance() {
            static ImagePreloader instance;
            return instance;
        }

private:
        ImagePreloader(): inv(false) {}
        ImagePreloader(const ImagePreloader& root) = delete;
        ImagePreloader& operator=(const ImagePreloader&) = delete;
};



#endif // IMAGEPRELOADER_H
