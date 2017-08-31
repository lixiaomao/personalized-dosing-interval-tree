

setwd("/workspace/daleks/FDI/simulation/results6/")
load("Simu_28.RData")

fdiTree<-function(i){
  folder="/workspace/daleks/FDI/simulation/results6/"
  setwd(folder)
  fm=paste0("pfmc_",i,".txt")
  fmc1=paste0("train_",i,".csv")
  fmc2=paste0("test_",i,".csv")
  if (grid.cv[i,"mode"]%in%c(1,2,5,6)){
    sided=1
    S=4.5
  } else {
    sided=0
    S=4.9
  }
  if (sum(list.files()==fm)>0){
    return("already there")
  } else if ((sum(list.files()==fmc1)>0) &&(sum(list.files()==fmc2)>0)){
    setwd("/workspace/daleks/FDI/fdiTree")
    system(paste0("fdiTree ",folder,fmc1," ",folder,fmc2," ",folder,fm," ",S," 0.5 ", sided," 15 5 10"))
    return("worked")
  } else {
    return("not prepared")
  }
}

setwd("/workspace/daleks/FDI/simulation/results6/")
load("Simu_1.RData")

library(parallel)
library(snowfall)

sfInit(parallel=TRUE, cpus=detectCores(), type="SOCK")

sfExportAll()

res <- sfClusterApplyLB(sample(1:dim(grid.cv)[1]), fdiTree)
