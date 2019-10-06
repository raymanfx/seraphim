FROM fedora

RUN dnf -y install make bash catch-devel ccache clang clang-tools-extra cmake \
    dlib-devel findutils git glfw-devel opencv-contrib opencv-devel \
    protobuf-compiler protobuf-devel qt5-qtbase-devel qt5-qtdeclarative-devel \
    qt5-qtmultimedia-devel qt5-qtquickcontrols2-devel && \
    dnf clean all && \
    rm -rf /var/cache/yum

RUN useradd -u 1000 builder

USER 1000
