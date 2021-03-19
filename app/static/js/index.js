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
            <div className="basic pd-6 br-1">
                <h3>基本选项</h3>
                <form>
                    <div>
                        <label htmlFor="use_agc">AGC: </label>
                        开启: <input type="radio" id="use_agc" name="use_agc" value={this.state.use_agc} />
                        关闭: <input type="radio" id="not_use_agc" name="use_agc" value={!this.state.use_agc} />
                    </div>
                    <div>
                        <label htmlFor="use_ns">NS: </label>
                        开启: <input type="radio" id="use_ns" name="use_ns" value={this.state.use_ns} />
                        关闭: <input type="radio" id="use_ns" name="use_ns" value={this.state.use_ns} />
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
            ns_level: 1
        };
    }

    render() {
        return (
            <form className="advance pd-6 br-1">
                <h3>高级选项</h3>
                <div>
                    <label htmlFor="in_mic">in_mic: </label>
                    <input type="text" id="in_mic" />
                </div>
                <div>
                    <label htmlFor="out_mic">out_mic: </label>
                    <input type="text" id="out_mic" />
                </div>
                <div>
                    <label htmlFor="agc_level">agc_level: </label>
                    <input type="text" id="agc_level" />
                </div>
                <div>
                    <label htmlFor="saturation_warning">saturation_warning: </label>
                    <input type="text" id="saturation_warning" />
                </div>
                <div>
                    <label htmlFor="compose_gain">compose_gain: </label>
                    <input type="text" id="compose_gain" />
                </div>
                <div>
                    <label htmlFor="target_level">target_level: </label>
                    <input type="text" id="target_level" />
                </div>
                <div>
                    <label htmlFor="limiter_enable">limiter_enable: </label>
                    <input type="text" id="limiter_enable" />
                </div>
                <div>
                    <label htmlFor="ns_level">ns_level: </label>
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
                {/* <image></image> */}
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
                <audio controls></audio>
                {/* <image></image> */}
            </div>
        )
    }
}

class UploadFile extends React.Component {
    render() {
        return (
            <div className="upload pd-6 br-1">
                <h1>Hi</h1>
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