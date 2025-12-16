# TTGL

### Prerequisites

- Install: `OpenCV`, `raylib`, `onnxruntime`
- Configure: `streams.csv`, `observed_areas.csv`
- Set up detection model:
  - Download YOLOv10 model with built-in top-k ([link](https://huggingface.co/onnx-community/yolov10m/blob/main/onnx/model.onnx))
  - Save it in project root as `model.onnx`

### Installation

#### Set up repository:

- Run `cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -S . -B build`

#### Build and run:

- Run `cmake --build build -j$(nproc)`
- Run `./app`

### TODO:

- Use hybrid detection minimizing compute-heavy AI use
- Implement tracking via image embeddings
- Add WebUI
- (QoL) upgrade to YOLOv11
- (QoL) compile onnxruntime with legacy GPU support

### Terms of use

THIS SOFTWARE MAY ONLY BE USED ON PRIVATE LAND WITH THE CONSENT OF THE PROPERTY OWNER. ALL VIDEO FEEDS MUST COMPLY WITH APPLICABLE LOCAL, REGIONAL, AND NATIONAL PRIVACY AND SURVEILLANCE LAWS. THE SOFTWARE IS PROVIDED FOR EDUCATIONAL PURPOSES ONLY AND IS NOT INTENDED FOR COMMERCIAL DEPLOYMENT OR LAW ENFORCEMENT USE.<br>
THIS SOFTWARE IS PROVIDED "AS IS," WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE, AND NON-INFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES, OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT, OR OTHERWISE, ARISING FROM, OUT OF, OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
