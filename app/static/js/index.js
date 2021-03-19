const app = document.querySelector('#app')

class BasicConfig extends React.Component {
    constructor(props) {
        super(props)
        this.state = {
            use_agc: true,
            use_ns: true
        };
    }

    render() {
        return (
            <div className="basic pd-6">
                <h3>基本选项</h3>
                <form>
                    <div>
                        <h4>AGC</h4>
                        <label htmlFor="use_agc">开启: </label>
                        <input type="radio" id="use_agc" name="use_agc" value={this.state.use_agc} />
                        <label htmlFor="not_use_agc">关闭: </label>
                        <input type="radio" id="not_use_agc" name="use_agc" value={!this.state.use_agc} />
                    </div>
                    <div>
                        <h4>NS</h4>
                        <label htmlFor="use_ns">开启: </label>
                        <input type="radio" id="use_ns" name="use_ns" value={this.state.use_ns} />
                        <label htmlFor="not_use_ns">关闭: </label>
                        <input type="radio" id="not_use_ns" name="use_ns" value={this.state.use_ns} />
                    </div>
                </form>
            </div>
        )
    }
}

class AdvanceConfig extends React.Component {
    constructor(props) {
        super(props)
        this.state = {
            in_mic: 0,
            out_mic: 255,
            agc_level: 2,
            saturation_warning: 1,
            compose_gain: 9,
            target_level: 3,
            limiter_enable: 1,
            ns_level: 1,
            enable: false
        };
    }

    render() {
        return (
            <form className="advance pd-6">
                <div>
                    <strong>高级选项</strong>
                    <input type="checkbox" checked={!this.state.enable}></input>
                </div>
                <div>
                    <h4><label htmlFor="in_mic">in_mic</label></h4>
                    <input type="text" id="in_mic" />
                </div>
                <div>
                    <h4><label htmlFor="out_mic">out_mic</label></h4>
                    <input type="text" id="out_mic" />
                </div>
                <div>
                    <h4><label htmlFor="agc_level">agc_level</label></h4>
                    <input type="text" id="agc_level" />
                </div>
                <div>
                    <h4><label htmlFor="saturation_warning">saturation_warning</label></h4>
                    <input type="text" id="saturation_warning" />
                </div>
                <div>
                    <h4><label htmlFor="compose_gain">compose_gain</label></h4>
                    <input type="text" id="compose_gain" />
                </div>
                <div>
                    <h4><label htmlFor="target_level">target_level</label></h4>
                    <input type="text" id="target_level" />
                </div>
                <div>
                    <h4><label htmlFor="limiter_enable">limiter_enable</label></h4>
                    <input type="text" id="limiter_enable" />
                </div>
                <div>
                    <h4><label htmlFor="ns_level">ns_level</label></h4>
                    <input type="text" id="ns_level" />
                </div>
            </form>
        )
    };
}

class OriginInfo extends React.Component {
    constructor(props) {
        super(props)
        this.state = {
            audio: props.audio,
            image: props.image
        }
    }

    render() {
        return (
            <div className="origin pd-6">
                <h3>原始数据</h3>
                <audio controls></audio>
                <img src={this.state.image} />
            </div>
        )
    }
}

class AfterInfo extends React.Component {
    constructor(props) {
        super(props)
        this.state = {
            audio: props.audio,
            image: props.image
        }
    }

    render() {
        return (
            <div className="after pd-6">
                <h3>处理后数据</h3>
                <div>
                    <audio controls></audio>
                </div>
                <div>
                    <img src={this.state.image} alt="处理后音频" />
                </div>
            </div>
        )
    }
}

class UploadFile extends React.Component {
    render() {
        return (
            <div className="upload pd-6">
                <input type="file"></input>
            </div>
        )
    };
}

class Container extends React.Component {
    render() {
        return (
            <div className="container">
                <BasicConfig />
                <AdvanceConfig />
                <OriginInfo />
                <AfterInfo />
                <UploadFile />
            </div>
        );
    }
}

ReactDOM.render(<Container />, app)