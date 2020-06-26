

void nominal_res() {
  TString fname = "src/Rt_BMG_6_1.dat";
  double radius, time, error;
  vector<double> v_radius, v_time, v_error = vector<double>();
  std::ifstream infile(fname);
  string line;
  int lineNum = 0;
  while (getline(infile, line)) {
    std::istringstream iss(line);
    if (lineNum != 0) {
      if (!(iss >> radius >> time >> error)) { break; }
      v_radius.push_back(radius);
      v_time  .push_back(time);
      v_error .push_back(error);
    }
    lineNum++;
  }

  TGraph* graph = new TGraph(v_error.size());
  for (int i = 0; i < v_error.size(); i++) {
    graph->SetPoint(i, v_radius.at(i), v_error.at(i));
  }

  graph->Fit("pol2");
  graph->Draw();
  gPad->Modified();
  gPad->Update();
  getchar();

}
