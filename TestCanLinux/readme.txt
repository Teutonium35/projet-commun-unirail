supprime le dossier build
rm -r build

creer le dossier build et va dans le dossier
mkdir build && cd build

sachant que le Cmakelists.txt est dans la racine du dossier
dans le dossier build
cmake -B. -H..
