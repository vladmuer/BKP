#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
}


MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_pushButton_clicked()
{
    QString str = QFileDialog::getOpenFileName(this, "Выберите файл для удаления", "C:\\");
    ui->lineEdit->setText(str);
}

void MainWindow::on_pushButton_2_clicked()
{
    QString PathToFile = ui->lineEdit->text();
    if(PathToFile != "" and SelectedMethod() != "")
    {
        QMessageBox::StandardButton sure = QMessageBox::question(this, "Delete File Tool", "Указанный файл будет удален. Вы уверены?", QMessageBox::Yes | QMessageBox::No);
        if(sure == QMessageBox::Yes)
        {
            bool isDeleted = MainWindow::DelFile(PathToFile.toLocal8Bit().data(), SelectedMethod());
            if(SelectedMethod() == "SSD" and isDeleted)
            {
                QMessageBox::information(this, "Успех", "Файл будет удален!");
                ui->lineEdit->setText("");
            }
            else if(SelectedMethod() != "SSD" and isDeleted)
            {
                QMessageBox::information(this, "Успех", "Файл успешно удален!");
                ui->lineEdit->setText("");
            }
            else
            {
                QMessageBox::critical(this, "Ошибка", "Файл не был удален!");
            }
        }
    }
    else if(PathToFile == "")
    {
        QMessageBox::critical(this, "Ошибка", "Выберите файл, который необходимо удалить");
    }
    else
    {
        QMessageBox::critical(this, "Ошибка", "Выберите метод уничтожения данных");
    }
}

bool MainWindow::TRIMstatus()
{
    system("fsutil behavior set disabledeletenotify NTFS 0 > 1.txt");
    string NTFS;
    ifstream in("1.txt");
    getline (in, NTFS);
    in.close();
    remove("1.txt");
    if (NTFS[27] == '0')
    {
        return true;
    }
    else
    {
        return false;
    }
}

string MainWindow::SelectedMethod()
{
    if(ui->t1SSD->isChecked())
    {
        return "SSD";
    }
    else if(ui->t2Nulls->isChecked())
    {
        return "Nulls";
    }
    else if(ui->t3Randoms->isChecked())
    {
        return "Randoms";
    }
    else if(ui->t4GOST->isChecked())
    {
        return "GOST";
    }
    else if(ui->t5NZSIT402->isChecked())
    {
        return "NZSIT402";
    }
    else if(ui->t6VSITR->isChecked())
    {
        return "VSITR";
    }
    else if(ui->t7DODmE->isChecked())
    {
        return "DOD5220.22-mE";
    }
    else if(ui->t8DODmECE->isChecked())
    {
        return "DOD5220.22-mECE";
    }
    else if(ui->t9NCSC->isChecked())
    {
        return "NCSC-TG-025";
    }
    else if(ui->t10AFSSI5020->isChecked())
    {
        return "AFSSI-5020";
    }
    else if(ui->t11Navso->isChecked())
    {
        return "NavsoP-5239-26";
    }
    else if(ui->t12AR38019->isChecked())
    {
        return "AR380-19";
    }
    else if(ui->t13CSEC->isChecked())
    {
        return "CSEC-ITSG-06";
    }
    else if(ui->t14HMGis5->isChecked())
    {
        return "HMGis5";
    }
    else if(ui->t15RCMP->isChecked())
    {
        return "RCMPtssitOPS-II";
    }
    else if(ui->t16Pfitzner7->isChecked())
    {
        return "Pfitzner7";
    }
    else if(ui->t17Pfitzner33->isChecked())
    {
        return "Pfitzner33";
    }
    else if(ui->t18Schneier->isChecked())
    {
        return "Schneier";
    }
    else if(ui->t19Gutmann->isChecked())
    {
        return "Gutmann";
    }
    else
    {
        return "";
    }
}

ULONGLONG* MainWindow::GetClusters(PCHAR file_name, ULONG size_of_cluster, ULONG *clusters_count, ULONG *file_size){
    HANDLE                        file_handle;
    ULONG                         output_buffer_size;
    ULONG                         output_bytes, k, cn_count, m;
    ULONGLONG                     *clusters = NULL;
    LARGE_INTEGER                 previous_VCN, Lcn;
    STARTING_VCN_INPUT_BUFFER     input_buffer;
    PRETRIEVAL_POINTERS_BUFFER    output_buffer;

    file_handle=CreateFileA(file_name, FILE_READ_ATTRIBUTES, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, NULL, OPEN_EXISTING, 0, 0);
    if (file_handle!=INVALID_HANDLE_VALUE){
        *file_size=GetFileSize(file_handle, NULL);
        output_buffer_size=sizeof(RETRIEVAL_POINTERS_BUFFER)+(*file_size/size_of_cluster)*sizeof(output_buffer->Extents);
        output_buffer=(PRETRIEVAL_POINTERS_BUFFER)malloc(output_buffer_size);
        input_buffer.StartingVcn.QuadPart=0;
        if (DeviceIoControl(file_handle, FSCTL_GET_RETRIEVAL_POINTERS, &input_buffer, sizeof(input_buffer), output_buffer, output_buffer_size, &output_bytes, NULL)){
            *clusters_count=(*file_size+size_of_cluster-1)/size_of_cluster;
            clusters=(ULONGLONG *)malloc(*clusters_count*sizeof(ULONGLONG));
            previous_VCN=output_buffer->StartingVcn;
            for (m=0, k=0; m<output_buffer->ExtentCount; m++){
                Lcn=output_buffer->Extents[m].Lcn;
                for (cn_count=(ULONG)(output_buffer->Extents[m].NextVcn.QuadPart-previous_VCN.QuadPart);
                     cn_count; cn_count--, k++, Lcn.QuadPart++) clusters[k]=Lcn.QuadPart;
                previous_VCN=output_buffer->Extents[m].NextVcn;
            }
        }
        free(output_buffer);
        CloseHandle(file_handle);
    }
    return clusters;
}


BOOL MainWindow::DelFile(PCHAR source_name, string type){
  ULONG         size_of_cluster, block_size;
  ULONGLONG     *clusters;
  ULONG         clusters_count, file_size, file_size_2, output_bytes;
  HANDLE        drive_handle, file_handle;
  ULONG         sectors_per_cluster, bytes_per_cluster, m, j;
  PVOID         buffer;
  LARGE_INTEGER offset;
  CHAR          drive[7];
  BOOL          isDeleted = true;
  CONST CHAR    *new_name;
  string        new_name_2, new_name_3;
  size_t        found;
  string        alphabet = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
  int           i;

  srand (time(0));
  drive[0]=source_name[0];
  drive[1]=':';
  drive[2]=0;
  GetDiskFreeSpaceA(drive, &sectors_per_cluster, &bytes_per_cluster, NULL, NULL);
  size_of_cluster=sectors_per_cluster * bytes_per_cluster;
  file_handle=CreateFileA(source_name, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_DELETE | FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, 0);
  if(file_handle == INVALID_HANDLE_VALUE)
  {
        isDeleted = false;
        return isDeleted;
  }
  CloseHandle(file_handle);
  clusters=MainWindow::GetClusters(source_name, size_of_cluster, &clusters_count, &file_size);
  ULONGLONG     PartSign[clusters_count], PartSign2[clusters_count];
  CHAR          str[size_of_cluster+2];
  if (clusters){
    drive[0]='\\';
    drive[1]='\\';
    drive[2]='.';
    drive[3]='\\';
    drive[4]=source_name[0];
    drive[5]=':';
    drive[6]=0;
    drive_handle = CreateFileA(drive, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, 0);
    if (drive_handle!=INVALID_HANDLE_VALUE){
      buffer=malloc(size_of_cluster);
      file_size_2 = file_size;
      for (m=0; m<clusters_count; m++, file_size_2-=block_size){
        offset.QuadPart=size_of_cluster*clusters[m];
        SetFilePointer(drive_handle, offset.LowPart, &offset.HighPart, FILE_BEGIN);
        ReadFile(drive_handle, buffer, size_of_cluster, &output_bytes, NULL);
        block_size=file_size_2 < size_of_cluster ? file_size_2 : size_of_cluster;
        PartSign[m] = *(ULONGLONG*) buffer;
      }
      free(buffer);
    }
    CloseHandle(drive_handle);
  }
  else{
    isDeleted = false;
    return isDeleted;
  }

  if (type == "Nulls"){
    file_handle=CreateFileA(source_name, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_DELETE | FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_ALWAYS, 0, 0);
    if (file_handle!=INVALID_HANDLE_VALUE){
      for(m=0; m<clusters_count; m++){
        memset(str, 0, size_of_cluster);
        WriteFile(file_handle, &str, size_of_cluster, NULL, NULL);
      }
    }
    CloseHandle(file_handle);
  }

  if (type == "Randoms" or type == "NZSIT402"){
    file_handle=CreateFileA(source_name, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_DELETE | FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_ALWAYS, 0, 0);
    if (file_handle!=INVALID_HANDLE_VALUE){
      for(m=0; m<clusters_count; m++){
        for (j=0; j<size_of_cluster; j++){
            str[j] = rand()%256;
        }
        WriteFile(file_handle, &str, size_of_cluster, NULL, NULL);
      }
    }
    CloseHandle(file_handle);
  }

  if (type == "GOST"){
    for (i=0; i<2; i++){
      file_handle=CreateFileA(source_name, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_DELETE | FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_ALWAYS, 0, 0);
      if (file_handle!=INVALID_HANDLE_VALUE){
        for(m=0; m<clusters_count; m++){
            for (j=0; j<size_of_cluster; j++){
                str[j] = rand()%256;
            }
            WriteFile(file_handle, &str, size_of_cluster, NULL, NULL);
        }
      }
      CloseHandle(file_handle);
    }
  }

  if (type == "VSITR"){
    for (j=0; j<3; j++){
      file_handle=CreateFileA(source_name, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_DELETE | FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_ALWAYS, 0, 0);
      if (file_handle!=INVALID_HANDLE_VALUE){
        for(m=0; m<clusters_count; m++){
            memset(str, 0, size_of_cluster);
            WriteFile(file_handle, &str, size_of_cluster, NULL, NULL);
        }
      }
      CloseHandle(file_handle);
      file_handle=CreateFileA(source_name, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_DELETE | FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_ALWAYS, 0, 0);
      if (file_handle!=INVALID_HANDLE_VALUE){
        for(m=0; m<clusters_count; m++){
            memset(str, 255, size_of_cluster);
            WriteFile(file_handle, &str, size_of_cluster, NULL, NULL);
        }
      }
      CloseHandle(file_handle);
    }
    file_handle=CreateFileA(source_name, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_DELETE | FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_ALWAYS, 0, 0);
    if (file_handle!=INVALID_HANDLE_VALUE){
      for(m=0; m<clusters_count; m++){
        memset(str, 170, size_of_cluster);
        WriteFile(file_handle, &str, size_of_cluster, NULL, NULL);
      }
    }
    CloseHandle(file_handle);
  }

  if (type == "DOD5220.22-mE" or type == "NCSC-TG-025" or type == "AFSSI-5020" or type == "NavsoP-5239-26" or type == "CSEC-ITSG-06" or type == "HMGis5"){
    file_handle=CreateFileA(source_name, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_DELETE | FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_ALWAYS, 0, 0);
    if (file_handle!=INVALID_HANDLE_VALUE){
      for(m=0; m<clusters_count; m++){
        memset(str, 0, size_of_cluster);
        WriteFile(file_handle, &str, size_of_cluster, NULL, NULL);
      }
    }
    CloseHandle(file_handle);
    file_handle=CreateFileA(source_name, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_DELETE | FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_ALWAYS, 0, 0);
    if (file_handle!=INVALID_HANDLE_VALUE){
      for(m=0; m<clusters_count; m++){
        memset(str, 255, size_of_cluster);
        WriteFile(file_handle, &str, size_of_cluster, NULL, NULL);
      }
    }
    CloseHandle(file_handle);
    file_handle=CreateFileA(source_name, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_DELETE | FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_ALWAYS, 0, 0);
    if (file_handle!=INVALID_HANDLE_VALUE){
      for(m=0; m<clusters_count; m++){
        for (j=0; j<size_of_cluster; j++){
            str[j] = rand()%256;
        }
        WriteFile(file_handle, &str, size_of_cluster, NULL, NULL);
      }
    }
    CloseHandle(file_handle);
  }

  if (type == "DOD5220.22-mECE"){
    file_handle=CreateFileA(source_name, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_DELETE | FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_ALWAYS, 0, 0);
    if (file_handle!=INVALID_HANDLE_VALUE){
      for(m=0; m<clusters_count; m++){
        memset(str, 0, size_of_cluster);
        WriteFile(file_handle, &str, size_of_cluster, NULL, NULL);
      }
    }
    CloseHandle(file_handle);
    file_handle=CreateFileA(source_name, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_DELETE | FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_ALWAYS, 0, 0);
    if (file_handle!=INVALID_HANDLE_VALUE){
      for(m=0; m<clusters_count; m++){
        memset(str, 255, size_of_cluster);
        WriteFile(file_handle, &str, size_of_cluster, NULL, NULL);
      }
    }
    CloseHandle(file_handle);
    for (i=0; i<2; i++){
      file_handle=CreateFileA(source_name, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_DELETE | FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_ALWAYS, 0, 0);
      if (file_handle!=INVALID_HANDLE_VALUE){
        for(m=0; m<clusters_count; m++){
            for (j=0; j<size_of_cluster; j++){
                str[j] = rand()%256;
            }
            WriteFile(file_handle, &str, size_of_cluster, NULL, NULL);
        }
      }
      CloseHandle(file_handle);
    }
    file_handle=CreateFileA(source_name, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_DELETE | FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_ALWAYS, 0, 0);
    if (file_handle!=INVALID_HANDLE_VALUE){
      for(m=0; m<clusters_count; m++){
        memset(str, 0, size_of_cluster);
        WriteFile(file_handle, &str, size_of_cluster, NULL, NULL);
      }
    }
    CloseHandle(file_handle);
    file_handle=CreateFileA(source_name, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_DELETE | FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_ALWAYS, 0, 0);
    if (file_handle!=INVALID_HANDLE_VALUE){
      for(m=0; m<clusters_count; m++){
        memset(str, 255, size_of_cluster);
        WriteFile(file_handle, &str, size_of_cluster, NULL, NULL);
      }
    }
    CloseHandle(file_handle);
    file_handle=CreateFileA(source_name, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_DELETE | FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_ALWAYS, 0, 0);
    if (file_handle!=INVALID_HANDLE_VALUE){
      for(m=0; m<clusters_count; m++){
        for (j=0; j<size_of_cluster; j++){
          str[j] = rand()%256;
        }
        WriteFile(file_handle, &str, size_of_cluster, NULL, NULL);
      }
    }
    CloseHandle(file_handle);
  }

  if (type == "RCMPtssitOPS-II"){
    for (j=0; j<3; j++){
      file_handle=CreateFileA(source_name, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_DELETE | FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_ALWAYS, 0, 0);
      if (file_handle!=INVALID_HANDLE_VALUE){
        for(m=0; m<clusters_count; m++){
          memset(str, 0, size_of_cluster);
          WriteFile(file_handle, &str, size_of_cluster, NULL, NULL);
        }
      }
      CloseHandle(file_handle);
      file_handle=CreateFileA(source_name, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_DELETE | FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_ALWAYS, 0, 0);
      if (file_handle!=INVALID_HANDLE_VALUE){
        for(m=0; m<clusters_count; m++){
          memset(str, 255, size_of_cluster);
          WriteFile(file_handle, &str, size_of_cluster, NULL, NULL);
        }
      }
      CloseHandle(file_handle);
    }
    file_handle=CreateFileA(source_name, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_DELETE | FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_ALWAYS, 0, 0);
    if (file_handle!=INVALID_HANDLE_VALUE){
      for(m=0; m<clusters_count; m++){
        for (j=0; j<size_of_cluster; j++){
          str[j] = rand()%256;
        }
        WriteFile(file_handle, &str, size_of_cluster, NULL, NULL);
      }
    }
    CloseHandle(file_handle);
  }

  if (type == "AR380-19"){
    file_handle=CreateFileA(source_name, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_DELETE | FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_ALWAYS, 0, 0);
    if (file_handle!=INVALID_HANDLE_VALUE){
      for(m=0; m<clusters_count; m++){
        for (j=0; j<size_of_cluster; j++){
          str[j] = rand()%256;
        }
        WriteFile(file_handle, &str, size_of_cluster, NULL, NULL);
      }
    }
    CloseHandle(file_handle);
    file_handle=CreateFileA(source_name, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_DELETE | FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_ALWAYS, 0, 0);
    if (file_handle!=INVALID_HANDLE_VALUE){
      for(m=0; m<clusters_count; m++){
        memset(str, 255, size_of_cluster);
        WriteFile(file_handle, &str, size_of_cluster, NULL, NULL);
      }
    }
    CloseHandle(file_handle);
    file_handle=CreateFileA(source_name, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_DELETE | FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_ALWAYS, 0, 0);
    if (file_handle!=INVALID_HANDLE_VALUE){
      for(m=0; m<clusters_count; m++){
        memset(str, 0, size_of_cluster);
        WriteFile(file_handle, &str, size_of_cluster, NULL, NULL);
      }
    }
    CloseHandle(file_handle);
  }

  if (type == "Pfitzner7"){
    for (i=0; i<7; i++){
      file_handle=CreateFileA(source_name, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_DELETE | FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_ALWAYS, 0, 0);
      if (file_handle!=INVALID_HANDLE_VALUE){
        for(m=0; m<clusters_count; m++){
          for (j=0; j<size_of_cluster; j++){
                str[j] = rand()%256;
          }
          WriteFile(file_handle, &str, size_of_cluster, NULL, NULL);
        }
      }
      CloseHandle(file_handle);
    }
  }

  if (type == "Pfitzner33"){
    for (i=0; i<33; i++){
      file_handle=CreateFileA(source_name, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_DELETE | FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_ALWAYS, 0, 0);
      if (file_handle!=INVALID_HANDLE_VALUE){
        for(m=0; m<clusters_count; m++){
          for (j=0; j<size_of_cluster; j++){
                str[j] = rand()%256;
          }
          WriteFile(file_handle, &str, size_of_cluster, NULL, NULL);
        }
      }
      CloseHandle(file_handle);
    }
  }

  if (type == "Schneier"){
    file_handle=CreateFileA(source_name, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_DELETE | FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_ALWAYS, 0, 0);
    if (file_handle!=INVALID_HANDLE_VALUE){
      for(m=0; m<clusters_count; m++){
        memset(str, 255, size_of_cluster);
        WriteFile(file_handle, &str, size_of_cluster, NULL, NULL);
      }
    }
    CloseHandle(file_handle);
    file_handle=CreateFileA(source_name, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_DELETE | FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_ALWAYS, 0, 0);
    if (file_handle!=INVALID_HANDLE_VALUE){
      for(m=0; m<clusters_count; m++){
        memset(str, 0, size_of_cluster);
        WriteFile(file_handle, &str, size_of_cluster, NULL, NULL);
      }
    }
    CloseHandle(file_handle);
    for (i=0; i<5; i++){
      file_handle=CreateFileA(source_name, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_DELETE | FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_ALWAYS, 0, 0);
      if (file_handle!=INVALID_HANDLE_VALUE){
        for(m=0; m<clusters_count; m++){
          for (j=0; j<size_of_cluster; j++){
                str[j] = rand()%256;
          }
          WriteFile(file_handle, &str, size_of_cluster, NULL, NULL);
        }
      }
      CloseHandle(file_handle);
    }
  }

  if (type == "Gutmann"){
    for (i=0; i<4; i++){
      file_handle=CreateFileA(source_name, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_DELETE | FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_ALWAYS, 0, 0);
      if (file_handle!=INVALID_HANDLE_VALUE){
        for(m=0; m<clusters_count; m++){
          for (j=0; j<size_of_cluster; j++){
                str[j] = rand()%256;
          }
          WriteFile(file_handle, &str, size_of_cluster, NULL, NULL);
        }
      }
      CloseHandle(file_handle);
    }
    file_handle=CreateFileA(source_name, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_DELETE | FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_ALWAYS, 0, 0);
    if (file_handle!=INVALID_HANDLE_VALUE){
      for(m=0; m<clusters_count; m++){
        memset(str, 85, size_of_cluster);
        WriteFile(file_handle, &str, size_of_cluster, NULL, NULL);
      }
    }
    CloseHandle(file_handle);
    file_handle=CreateFileA(source_name, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_DELETE | FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_ALWAYS, 0, 0);
    if (file_handle!=INVALID_HANDLE_VALUE){
      for(m=0; m<clusters_count; m++){
        memset(str, 170, size_of_cluster);
        WriteFile(file_handle, &str, size_of_cluster, NULL, NULL);
      }
    }
    CloseHandle(file_handle);
    file_handle=CreateFileA(source_name, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_DELETE | FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_ALWAYS, 0, 0);
    if (file_handle!=INVALID_HANDLE_VALUE){
      for (m=0; m<size_of_cluster; m+=3){
        str[m] = 146;
        str[m+1] = 73;
        str[m+2] = 36;
      }
      for(m=0; m<clusters_count; m++){
        WriteFile(file_handle, &str, size_of_cluster, NULL, NULL);
      }
    }
    CloseHandle(file_handle);
    file_handle=CreateFileA(source_name, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_DELETE | FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_ALWAYS, 0, 0);
    if (file_handle!=INVALID_HANDLE_VALUE){
      for (m=0; m<size_of_cluster; m+=3){
        str[m] = 73;
        str[m+1] = 36;
        str[m+2] = 146;
      }
      for(m=0; m<clusters_count; m++){
        WriteFile(file_handle, &str, size_of_cluster, NULL, NULL);
      }
    }
    CloseHandle(file_handle);
    file_handle=CreateFileA(source_name, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_DELETE | FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_ALWAYS, 0, 0);
    if (file_handle!=INVALID_HANDLE_VALUE){
      for (m=0; m<size_of_cluster; m+=3){
        str[m] = 36;
        str[m+1] = 146;
        str[m+2] = 73;
      }
      for(m=0; m<clusters_count; m++){
        WriteFile(file_handle, &str, size_of_cluster, NULL, NULL);
      }
    }
    CloseHandle(file_handle);
    file_handle=CreateFileA(source_name, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_DELETE | FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_ALWAYS, 0, 0);
    if (file_handle!=INVALID_HANDLE_VALUE){
      for(m=0; m<clusters_count; m++){
        memset(str, 0, size_of_cluster);
        WriteFile(file_handle, &str, size_of_cluster, NULL, NULL);
      }
    }
    CloseHandle(file_handle);
    file_handle=CreateFileA(source_name, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_DELETE | FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_ALWAYS, 0, 0);
    if (file_handle!=INVALID_HANDLE_VALUE){
      for(m=0; m<clusters_count; m++){
        memset(str, 17, size_of_cluster);
        WriteFile(file_handle, &str, size_of_cluster, NULL, NULL);
      }
    }
    CloseHandle(file_handle);
    file_handle=CreateFileA(source_name, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_DELETE | FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_ALWAYS, 0, 0);
    if (file_handle!=INVALID_HANDLE_VALUE){
      for(m=0; m<clusters_count; m++){
        memset(str, 34, size_of_cluster);
        WriteFile(file_handle, &str, size_of_cluster, NULL, NULL);
      }
    }
    CloseHandle(file_handle);
    file_handle=CreateFileA(source_name, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_DELETE | FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_ALWAYS, 0, 0);
    if (file_handle!=INVALID_HANDLE_VALUE){
      for(m=0; m<clusters_count; m++){
        memset(str, 51, size_of_cluster);
        WriteFile(file_handle, &str, size_of_cluster, NULL, NULL);
      }
    }
    CloseHandle(file_handle);
    file_handle=CreateFileA(source_name, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_DELETE | FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_ALWAYS, 0, 0);
    if (file_handle!=INVALID_HANDLE_VALUE){
      for(m=0; m<clusters_count; m++){
        memset(str, 68, size_of_cluster);
        WriteFile(file_handle, &str, size_of_cluster, NULL, NULL);
      }
    }
    CloseHandle(file_handle);
    file_handle=CreateFileA(source_name, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_DELETE | FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_ALWAYS, 0, 0);
    if (file_handle!=INVALID_HANDLE_VALUE){
      for(m=0; m<clusters_count; m++){
        memset(str, 85, size_of_cluster);
        WriteFile(file_handle, &str, size_of_cluster, NULL, NULL);
      }
    }
    CloseHandle(file_handle);
    file_handle=CreateFileA(source_name, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_DELETE | FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_ALWAYS, 0, 0);
    if (file_handle!=INVALID_HANDLE_VALUE){
      for(m=0; m<clusters_count; m++){
        memset(str, 102, size_of_cluster);
        WriteFile(file_handle, &str, size_of_cluster, NULL, NULL);
      }
    }
    CloseHandle(file_handle);
    file_handle=CreateFileA(source_name, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_DELETE | FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_ALWAYS, 0, 0);
    if (file_handle!=INVALID_HANDLE_VALUE){
      for(m=0; m<clusters_count; m++){
        memset(str, 119, size_of_cluster);
        WriteFile(file_handle, &str, size_of_cluster, NULL, NULL);
      }
    }
    CloseHandle(file_handle);
    file_handle=CreateFileA(source_name, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_DELETE | FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_ALWAYS, 0, 0);
    if (file_handle!=INVALID_HANDLE_VALUE){
      for(m=0; m<clusters_count; m++){
        memset(str, 136, size_of_cluster);
        WriteFile(file_handle, &str, size_of_cluster, NULL, NULL);
      }
    }
    CloseHandle(file_handle);
    file_handle=CreateFileA(source_name, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_DELETE | FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_ALWAYS, 0, 0);
    if (file_handle!=INVALID_HANDLE_VALUE){
      for(m=0; m<clusters_count; m++){
        memset(str, 153, size_of_cluster);
        WriteFile(file_handle, &str, size_of_cluster, NULL, NULL);
      }
    }
    CloseHandle(file_handle);
    file_handle=CreateFileA(source_name, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_DELETE | FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_ALWAYS, 0, 0);
    if (file_handle!=INVALID_HANDLE_VALUE){
      for(m=0; m<clusters_count; m++){
        memset(str, 170, size_of_cluster);
        WriteFile(file_handle, &str, size_of_cluster, NULL, NULL);
      }
    }
    CloseHandle(file_handle);
    file_handle=CreateFileA(source_name, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_DELETE | FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_ALWAYS, 0, 0);
    if (file_handle!=INVALID_HANDLE_VALUE){
      for(m=0; m<clusters_count; m++){
        memset(str, 187, size_of_cluster);
        WriteFile(file_handle, &str, size_of_cluster, NULL, NULL);
      }
    }
    CloseHandle(file_handle);
    file_handle=CreateFileA(source_name, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_DELETE | FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_ALWAYS, 0, 0);
    if (file_handle!=INVALID_HANDLE_VALUE){
      for(m=0; m<clusters_count; m++){
        memset(str, 204, size_of_cluster);
        WriteFile(file_handle, &str, size_of_cluster, NULL, NULL);
      }
    }
    CloseHandle(file_handle);
    file_handle=CreateFileA(source_name, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_DELETE | FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_ALWAYS, 0, 0);
    if (file_handle!=INVALID_HANDLE_VALUE){
      for(m=0; m<clusters_count; m++){
        memset(str, 221, size_of_cluster);
        WriteFile(file_handle, &str, size_of_cluster, NULL, NULL);
      }
    }
    CloseHandle(file_handle);
    file_handle=CreateFileA(source_name, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_DELETE | FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_ALWAYS, 0, 0);
    if (file_handle!=INVALID_HANDLE_VALUE){
      for(m=0; m<clusters_count; m++){
        memset(str, 238, size_of_cluster);
        WriteFile(file_handle, &str, size_of_cluster, NULL, NULL);
      }
    }
    CloseHandle(file_handle);
    file_handle=CreateFileA(source_name, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_DELETE | FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_ALWAYS, 0, 0);
    if (file_handle!=INVALID_HANDLE_VALUE){
      for(m=0; m<clusters_count; m++){
        memset(str, 255, size_of_cluster);
        WriteFile(file_handle, &str, size_of_cluster, NULL, NULL);
      }
    }
    CloseHandle(file_handle);
    file_handle=CreateFileA(source_name, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_DELETE | FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_ALWAYS, 0, 0);
    if (file_handle!=INVALID_HANDLE_VALUE){
      for (m=0; m<size_of_cluster; m+=3){
        str[m] = 146;
        str[m+1] = 73;
        str[m+2] = 36;
      }
      for(m=0; m<clusters_count; m++){
        WriteFile(file_handle, &str, size_of_cluster, NULL, NULL);
      }
    }
    CloseHandle(file_handle);
    file_handle=CreateFileA(source_name, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_DELETE | FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_ALWAYS, 0, 0);
    if (file_handle!=INVALID_HANDLE_VALUE){
      for (m=0; m<size_of_cluster; m+=3){
        str[m] = 73;
        str[m+1] = 36;
        str[m+2] = 146;
      }
      for(m=0; m<clusters_count; m++){
        WriteFile(file_handle, &str, size_of_cluster, NULL, NULL);
      }
    }
    CloseHandle(file_handle);
    file_handle=CreateFileA(source_name, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_DELETE | FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_ALWAYS, 0, 0);
    if (file_handle!=INVALID_HANDLE_VALUE){
      for (m=0; m<size_of_cluster; m+=3){
        str[m] = 36;
        str[m+1] = 146;
        str[m+2] = 73;
      }
      for(m=0; m<clusters_count; m++){
        WriteFile(file_handle, &str, size_of_cluster, NULL, NULL);
      }
    }
    CloseHandle(file_handle);
    file_handle=CreateFileA(source_name, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_DELETE | FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_ALWAYS, 0, 0);
    if (file_handle!=INVALID_HANDLE_VALUE){
      for (m=0; m<size_of_cluster; m+=3){
        str[m] = 109;
        str[m+1] = 182;
        str[m+2] = 219;
      }
      for(m=0; m<clusters_count; m++){
        WriteFile(file_handle, &str, size_of_cluster, NULL, NULL);
      }
    }
    CloseHandle(file_handle);
    file_handle=CreateFileA(source_name, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_DELETE | FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_ALWAYS, 0, 0);
    if (file_handle!=INVALID_HANDLE_VALUE){
      for (m=0; m<size_of_cluster; m+=3){
        str[m] = 182;
        str[m+1] = 219;
        str[m+2] = 109;
      }
      for(m=0; m<clusters_count; m++){
        WriteFile(file_handle, &str, size_of_cluster, NULL, NULL);
      }
    }
    CloseHandle(file_handle);
    file_handle=CreateFileA(source_name, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_DELETE | FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_ALWAYS, 0, 0);
    if (file_handle!=INVALID_HANDLE_VALUE){
      for (m=0; m<size_of_cluster; m+=3){
        str[m] = 219;
        str[m+1] = 109;
        str[m+2] = 182;
      }
      for(m=0; m<clusters_count; m++){
        WriteFile(file_handle, &str, size_of_cluster, NULL, NULL);
      }
    }
    CloseHandle(file_handle);
    for (i=0; i<4; i++){
      file_handle=CreateFileA(source_name, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_DELETE | FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_ALWAYS, 0, 0);
      if (file_handle!=INVALID_HANDLE_VALUE){
        for(m=0; m<clusters_count; m++){
          for (j=0; j<size_of_cluster; j++){
                str[j] = rand()%256;
          }
          WriteFile(file_handle, &str, size_of_cluster, NULL, NULL);
        }
      }
      CloseHandle(file_handle);
    }
  }

  drive_handle = CreateFileA(drive, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, 0);
  if (drive_handle!=INVALID_HANDLE_VALUE){
      buffer=malloc(size_of_cluster);
      file_size_2 = file_size;
      block_size = 0;
      for (m=0; m<clusters_count; m++, file_size_2-=block_size){
        offset.QuadPart=size_of_cluster*clusters[m];
        SetFilePointer(drive_handle, offset.LowPart, &offset.HighPart, FILE_BEGIN);
        ReadFile(drive_handle, buffer, size_of_cluster, &output_bytes, NULL);
        block_size=file_size_2 < size_of_cluster ? file_size_2 : size_of_cluster;
        PartSign2[m] = *(ULONGLONG*) buffer;
      }
      free(buffer);
  }
  CloseHandle(drive_handle);

  for (m=0; m<clusters_count; m++){
    if (PartSign[m] == PartSign2[m]){
        isDeleted = false;
    }
    PartSign[m] = 0;
    PartSign2[m] = 0;
  };

  if (type == "SSD"){
    if (MainWindow::TRIMstatus()){
      isDeleted = true;
    }
  }

  free(clusters);
  if (isDeleted){
    new_name_2 = source_name;
    found = new_name_2.find_last_of("/\\");
    for (int j=0; j<((rand()%10)+1); j++){
        new_name_3 += alphabet[rand()%62];
    };
    new_name_2 = new_name_2.substr(0,found) + "/" + new_name_3 + ".exe";
    new_name = new_name_2.c_str();
    rename(source_name, new_name);
    remove(new_name);
  }
  return isDeleted;
}
