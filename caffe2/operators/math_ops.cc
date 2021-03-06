#include "caffe2/operators/math_ops.h"
#include "caffe2/utils/math.h"


namespace caffe2 {

struct LogCPUFunctor {
  template <typename T>
  inline void
  operator()(const int n, const T* x, T* y, CPUContext* device_context) {
    math::Log<T, CPUContext>(n, x, y, device_context);
  }
};

struct SqrCPUFunctor {
  template <typename T>
  inline void
  operator()(const int n, const T* x, T* y, CPUContext* device_context) {
    math::Sqr<T, CPUContext>(n, x, y, device_context);
  }
};

namespace {
REGISTER_CPU_OPERATOR(
    Log,
    UnaryElementwiseOp<TensorTypes<float>, CPUContext, LogCPUFunctor>);
REGISTER_CPU_OPERATOR(
    Sqr,
    UnaryElementwiseOp<TensorTypes<float>, CPUContext, SqrCPUFunctor>);

OPERATOR_SCHEMA(Log)
    .NumInputs(1)
    .NumOutputs(1)
    .AllowInplace({{0, 0}})
    .IdenticalTypeAndShape()
    .SetDoc(R"DOC(
Calculates the natural log of the given input tensor, element-wise. This
operation can be done in an in-place fashion too, by providing the same input
and output blobs.
)DOC")
    .Input(0, "input", "Input tensor")
    .Output(
        0,
        "output",
        "The natural log of the input tensor computed "
        "element-wise");

OPERATOR_SCHEMA(Sqr)
    .NumInputs(1)
    .NumOutputs(1)
    .AllowInplace({{0, 0}})
    .IdenticalTypeAndShape()
    .SetDoc("Square (x^2) the elements of the input")
    .Input(0, "input", "Input tensor")
    .Output(0, "output", "Squared elements of the input");

class GetLogGradient : public GradientMakerBase {
  using GradientMakerBase::GradientMakerBase;
  vector<OperatorDef> GetGradientDefs() override {
    return SingleGradientDef(
        "Div",
        "",
        std::vector<string>{GO(0), I(0)},
        std::vector<string>{GI(0)});
  }
};
REGISTER_GRADIENT(Log, GetLogGradient);

class GetSqrGradient : public GradientMakerBase {
  using GradientMakerBase::GradientMakerBase;
  vector<OperatorDef> GetGradientDefs() override {
    Argument scale_arg;
    scale_arg.set_name("scale");
    scale_arg.set_f(2.0);
    return vector<OperatorDef>{CreateOperatorDef(
                                   "Scale",
                                   "",
                                   std::vector<string>{GO(0)},
                                   std::vector<string>{GO(0)},
                                   std::vector<Argument>{scale_arg}),
                               CreateOperatorDef(
                                   "Mul",
                                   "",
                                   std::vector<string>{GO(0), I(0)},
                                   std::vector<string>{GI(0)})};
  }
};
REGISTER_GRADIENT(Sqr, GetSqrGradient);

REGISTER_CPU_OPERATOR(
    Pow,
    UnaryElementwiseWithArgsOp<TensorTypes<float>, CPUContext, PowFunctor>);

OPERATOR_SCHEMA(Pow)
    .NumInputs(1)
    .NumOutputs(1)
    .Arg("exponent", "The exponent of the power function.")
    .AllowInplace({{0, 0}})
    .IdenticalTypeAndShape()
    .SetDoc(R"DOC(
Pow takes input data (Tensor<T>) and an argument exponent, and
produces one output data (Tensor<T>) where the function `f(x) = x^exponent`,
is applied to the data tensor elementwise.
)DOC")
    .Input(0, "X", "Input tensor of any shape")
    .Output(0, "Y", "Output tensor (same size as X)");

class GetPowGradient : public GradientMakerBase {
  using GradientMakerBase::GradientMakerBase;
  vector<OperatorDef> GetGradientDefs() override {
    ArgumentHelper arg_helper(def_);
    float exponent = arg_helper.GetSingleArgument<float>("exponent", 0.0);
    Argument scale_arg;
    scale_arg.set_name("scale");
    scale_arg.set_f(exponent);
    Argument pow_arg;
    pow_arg.set_name("exponent");
    pow_arg.set_f(exponent - 1);
    return vector<OperatorDef>{CreateOperatorDef(
                                   "Pow",
                                   "",
                                   std::vector<string>{I(0)},
                                   std::vector<string>{GI(0)},
                                   std::vector<Argument>{pow_arg}),
                               CreateOperatorDef(
                                   "Mul",
                                   "",
                                   std::vector<string>{GI(0), GO(0)},
                                   std::vector<string>{GI(0)}),
                               CreateOperatorDef(
                                   "Scale",
                                   "",
                                   std::vector<string>{GI(0)},
                                   std::vector<string>{GI(0)},
                                   std::vector<Argument>{scale_arg})};
  }
  virtual bool CopyArguments() const override {
    return false;
  }
};

REGISTER_GRADIENT(Pow, GetPowGradient);

} // namespace
} // namespace caffe2
