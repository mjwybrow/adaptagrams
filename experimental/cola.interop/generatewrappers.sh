ColaClassLibraryPath="c:\\Users\\t-tdwyer\\Documents\\devel\\ColaClassLibrary\\ColaClassLibrary"
ColaPath="c:\\Users\\t-tdwyer\\Documents\\devel\\cola"
pushd /cygdrive/c/Users/t-tdwyer/Documents/devel/WpfGraphEditor/WpfGraphEditor/cola 
rm *.cs
popd

/cygdrive/c/Program\ Files/swigwin-1.3.36/swig -v -csharp -namespace org.adaptagrams.cola -outdir $ColaClassLibraryPath -c++ $ColaPath\\cola.i

pushd $ColaClassLibraryPath
for f in *.cs
do
sed -e's/std::pair< unsigned int,double >/OffsetPair/' \
    -e's/topology::Edge/Edge/' \
    -e's/topology::Node/Node/' \
    -e's/vpsc::Rectangle/Rectangle/' \
    -e's/cola::Resize/Resize/' \
    -e's/cola::Edge/ColaEdge/' \
    -e's/cola::Lock/Lock/' \
    -e's/cola::CompoundConstraint/CompoundConstraint/' \
    -e's/cola::Cluster/Cluster/' \
    -e's/SWIGTYPE_p_RootCluster/RootCluster/g' \
$f > $f.1
mv $f.1 $f
done
popd
